#include "memory.hpp"

bool memory_t::is_memory_valid(uintptr_t address)
{
    MEMORY_BASIC_INFORMATION mem_info;
    if (VirtualQueryEx(this->process_handle, reinterpret_cast<const void*>(address), &mem_info, sizeof(mem_info)) == sizeof(mem_info))
        return mem_info.State == MEM_COMMIT && (mem_info.Type == MEM_PRIVATE || mem_info.Type == MEM_MAPPED);

    return false;
}

bool memory_t::is_page_in_phys(uintptr_t address)
{
    PSAPI_WORKING_SET_EX_INFORMATION working_set_info;
    working_set_info.VirtualAddress = reinterpret_cast<void*>(address);

    if (K32QueryWorkingSetEx(this->process_handle, &working_set_info, sizeof(working_set_info)))
        return working_set_info.VirtualAttributes.Valid;
    else
    {
        throw new std::runtime_error("Failed to query working set.");
        return false;
    }
}

void memory_t::attach(std::optional<DWORD> process_id)
{
    if (this->process_handle)
    {
        CloseHandle(this->process_handle);
        this->process_handle = nullptr;
    }

    if (process_id.has_value())
    {
        auto handle = this->hijack_handle(process_id.value());
        if (!handle.has_value())
            throw new std::runtime_error("Failed to hijack handle.");

        this->process_handle = handle.value();
        this->process_id = process_id;
        this->is_attached = true;
    }
    else
        throw new std::runtime_error("Invalid process ID provided.");
}

std::optional<HANDLE> memory_t::hijack_handle(std::optional<DWORD> process_id)
{
    const auto get_proc_address_s = [](HMODULE hModule, LPCSTR lpProcName) -> FARPROC
        {
            const auto address = GetProcAddress(hModule, lpProcName);
            if (!address)
                throw new std::runtime_error("Failed to get function address.");

            return address;
        };

    const auto mod = GetModuleHandleA("ntdll.dll");
    if (!mod)
    {
        throw new std::runtime_error("Failed to get ntdll.dll module handle.");
        return std::nullopt;
    }

    static const auto RtlAdjustPrivilege = reinterpret_cast<_RtlAdjustPrivilege>(get_proc_address_s(mod, "RtlAdjustPrivilege"));
    static const auto NtQuerySystemInformation = reinterpret_cast<_NtQuerySystemInformation>(get_proc_address_s(mod, "NtQuerySystemInformation"));
    static const auto NtDuplicateObject = reinterpret_cast<_NtDuplicateObject>(get_proc_address_s(mod, "NtDuplicateObject"));
    static const auto NtOpenProcess = reinterpret_cast<_NtOpenProcess>(get_proc_address_s(mod, "NtOpenProcess"));

    if (!RtlAdjustPrivilege || !NtQuerySystemInformation || !NtDuplicateObject || !NtOpenProcess)
    {
        throw new std::runtime_error("Failed to query function addresses.");
        return std::nullopt;
    }

    BOOLEAN old_priv;
    RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &old_priv);

    DWORD buffer_size = sizeof(SYSTEM_HANDLE_INFORMATION);
    SYSTEM_HANDLE_INFORMATION* handle_info = nullptr;

    NTSTATUS status;
    do
    {
        delete[] handle_info;
        buffer_size *= 2;
        handle_info = reinterpret_cast<SYSTEM_HANDLE_INFORMATION*>(new BYTE[buffer_size]);
        status = NtQuerySystemInformation(SystemHandleInformation, handle_info, buffer_size, 0);
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    if (!NT_SUCCESS(status))
    {
        delete[] handle_info;
        throw new std::runtime_error("NtQuerySystemInformation failed.");
        return std::nullopt;
    }

    HANDLE process_handle = nullptr;
    for (auto i = 0ul; i < handle_info->HandleCount; ++i)
    {
        auto handle = handle_info->Handles[i];
        if (handle.ObjectTypeNumber != ProcessHandleType)
            continue;

        HANDLE source_process_handle;
        CLIENT_ID client_id = { (HANDLE)handle.ProcessId, nullptr };
        OBJECT_ATTRIBUTES object_attributes = { sizeof(OBJECT_ATTRIBUTES) };

        status = NtOpenProcess(&source_process_handle, PROCESS_DUP_HANDLE, &object_attributes, &client_id);
        if (!NT_SUCCESS(status))
            continue;

        HANDLE target_handle;
        status = NtDuplicateObject(source_process_handle, (HANDLE)handle.Handle, GetCurrentProcess(), &target_handle, PROCESS_ALL_ACCESS, 0, 0);
        CloseHandle(source_process_handle);

        if (!NT_SUCCESS(status))
            continue;

        if (GetProcessId(target_handle) == process_id)
        {
            process_handle = target_handle;
            break;
        }

        CloseHandle(target_handle);
    }

    delete[] handle_info;

    if (!process_handle)
    {
        throw new std::runtime_error("Failed to find valid handle.");
        return std::nullopt;
    }
    return process_handle;
}