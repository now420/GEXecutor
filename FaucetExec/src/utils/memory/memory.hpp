#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <optional>
#include <thread>

#include "WinAPI.hpp"

class memory_t
{
private:
    std::optional<DWORD> process_id;
    HANDLE process_handle = nullptr;  // Initialize process_handle
    bool is_attached = false;         // Initialize is_attached

    bool is_memory_valid(uintptr_t address);
    bool is_page_in_phys(uintptr_t address);

    std::optional<HANDLE> hijack_handle(std::optional<DWORD> process_id);

public:
public:
    template <class Ty>
    Ty read(const uintptr_t address)
    {
        Ty value = {};
        for (auto i = 0; i < 5;)
        {
            if (!this->is_memory_valid(address) || !this->is_page_in_phys(address))
            {
                i++;
            }
            else
                break;
        }

        MEMORY_BASIC_INFORMATION mem_basic_info;

        static auto NtUnlockVirtualMemory = reinterpret_cast<_NtUnlockVirtualMemory*>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtUnlockVirtualMemory"));
        NtUnlockVirtualMemory(this->process_handle, &mem_basic_info.AllocationBase, &mem_basic_info.RegionSize, 1);

        VirtualQueryEx(this->process_handle, reinterpret_cast<const void*>(address), &mem_basic_info, sizeof(MEMORY_BASIC_INFORMATION));
        ReadProcessMemory(this->process_handle, reinterpret_cast<const void*>(address), &value, sizeof(Ty), nullptr);

        NtUnlockVirtualMemory(this->process_handle, &mem_basic_info.AllocationBase, &mem_basic_info.RegionSize, 1);

        return value;
    }

    template <class Ty>
    void write(const uintptr_t address, const Ty& value)
    {
        for (auto i = 0; i < 5;)
        {
            if (!this->is_memory_valid(address) || !this->is_page_in_phys(address))
            {
                i++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else
                break;
        }

        MEMORY_BASIC_INFORMATION mem_basic_info;

        VirtualQueryEx(this->process_handle, reinterpret_cast<const void*>(address), &mem_basic_info, sizeof(MEMORY_BASIC_INFORMATION));
        WriteProcessMemory(this->process_handle, reinterpret_cast<LPVOID>(address), &value, sizeof(Ty), nullptr);

        static auto NtUnlockVirtualMemory = reinterpret_cast<_NtUnlockVirtualMemory*>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtUnlockVirtualMemory"));
        NtUnlockVirtualMemory(this->process_handle, &mem_basic_info.AllocationBase, &mem_basic_info.RegionSize, 1);
    }

    uintptr_t allocate_virtual_memory(size_t size, DWORD allocation_type = MEM_COMMIT | MEM_RESERVE, DWORD protect = PAGE_READWRITE) {
        if (!process_handle) {
            std::cerr << "Process handle not initialized." << std::endl;
            return 0;
        }

        // Allocate memory in the target process
        void* allocated_memory = VirtualAllocEx(process_handle, nullptr, size, allocation_type, protect);
        if (allocated_memory == nullptr) {
            std::cerr << "Failed to allocate virtual memory. Error: " << GetLastError() << std::endl;
            return 0;
        }

        return reinterpret_cast<uintptr_t>(allocated_memory);
    }

    template <typename T>
    void write_memory(const uintptr_t destination, const T* source, const size_t size) {
        for (int attempt = 0; attempt < 5; ++attempt) {
            if (!this->is_memory_valid(destination) || !this->is_page_in_phys(destination)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else {
                break;
            }
        }

        // Query memory info before writing
        MEMORY_BASIC_INFORMATION mem_basic_info;
        VirtualQueryEx(this->process_handle, reinterpret_cast<const void*>(destination), &mem_basic_info, sizeof(MEMORY_BASIC_INFORMATION));

        // Perform the write operation with the specified source and size
        WriteProcessMemory(this->process_handle, reinterpret_cast<LPVOID>(destination), reinterpret_cast<LPCVOID>(source), size, nullptr);

        // Unlock memory using NtUnlockVirtualMemory if available
        static auto NtUnlockVirtualMemory = reinterpret_cast<_NtUnlockVirtualMemory*>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtUnlockVirtualMemory"));
        if (NtUnlockVirtualMemory) {
            NtUnlockVirtualMemory(this->process_handle, &mem_basic_info.AllocationBase, &mem_basic_info.RegionSize, 1);
        }
    }


    void attach(std::optional<DWORD> process_id);

    // Public getter methods
    HANDLE get_process_handle() const { return process_handle; }
    bool is_memory_valid_public(uintptr_t address) { return is_memory_valid(address); }
    bool is_page_in_phys_public(uintptr_t address) { return is_page_in_phys(address); }
};

inline auto memory = std::make_unique<memory_t>();