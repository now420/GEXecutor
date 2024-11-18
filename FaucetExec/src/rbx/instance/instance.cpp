#include "instance.hpp"
#include "../../utils/memory/memory.hpp"
#include "..\offsets\offsets.hpp"


std::string rbxstring(std::uintptr_t address)
{
	const auto size = memory->read<size_t>(address + 0x10);

	if (size >= 16)
		address = memory->read<std::uintptr_t>(address);

	std::string str;

	BYTE c = 0;

	for (std::int32_t i = 0; c = memory->read<std::uint8_t>(address + i); i++)
		str.push_back(c);

	return str;
}

void rbx::instance_t::SetBytecode(std::vector<char> bytes, int bytecode_size)
{
    auto old_bytecode_ptr = memory->read<long long>(this->address + offsets::script::msbytecode);

    auto protected_str_ptr = (long long)memory->allocate_virtual_memory(bytecode_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    /* Allocated nice */

    memory->write_memory(protected_str_ptr, bytes.data(), bytes.size());
    memory->write<unsigned long long>(old_bytecode_ptr + 0x10, protected_str_ptr);
    memory->write<unsigned long>(old_bytecode_ptr + 0x20, bytecode_size);
}

template <typename T>
void mem_read(DWORD64 address, T* buffer, SIZE_T size = 0) {
    if (size == 0) {
        size = sizeof(T);
    }

    ReadProcessMemory(memory->get_process_handle(), (LPCVOID)address, buffer, size, NULL);
}

std::vector<char> read_bytes(DWORD64 address, SIZE_T size = 500) {
    std::vector<char> buffer(size, 0);
    mem_read(address, buffer.data(), size);

    return buffer;
}

void rbx::instance_t::GetBytecode(std::vector<char>& bytecode, size_t& bytecode_size) {
    DWORD64 bytecode_pointer;
    auto meow = this->getclassname();
    if (meow == "LocalScript") {
        bytecode_pointer = memory->read<long long>(this->address + offsets::script::lsbytecode);
    }
    else if (meow == "ModuleScript") {
        bytecode_pointer = memory->read<long long>(this->address + offsets::script::msbytecode);
    }
    else {
        return;
    }

    if (bytecode_pointer > 1000) {
        bytecode = read_bytes(bytecode_pointer + 0x10);
        mem_read(bytecode_pointer + 0x20, &bytecode_size);
    };
}

void rbx::instance_t::modulebypassi() {
    uint64_t set = 0x100000000;
    uint64_t core = 0x1;

    memory->write(address + offsets::script::moduleflags, set);
    memory->write(address + offsets::script::iscore, core);
}

std::vector<rbx::instance_t> rbx::instance_t::getchildren()
{
    std::vector<rbx::instance_t> container;

    if (!this->address)
        return container;

    auto start = memory->read<std::uint64_t>(this->address + offsets::instance::children);

    if (!start)
        return container;

    auto end = memory->read<std::uint64_t>(start + offsets::instance::childsize);

    for (auto instances = memory->read<std::uint64_t>(start); instances != end; instances += 16) {
        rbx::instance_t aee = memory->read<rbx::instance_t>(instances);
        container.emplace_back(aee);
    }


    return container;
}

rbx::instance_t rbx::instance_t::findfirstchild(std::string name)
{
    rbx::instance_t ret;

    for (auto& object : this->getchildren())
    {
        if (object.getname() == name)
        {
            ret = static_cast<rbx::instance_t>(object);
            break;
        }
    }

    return ret;
}

rbx::instance_t rbx::instance_t::ObjectValue()
{
    rbx::instance_t ret = memory->read<rbx::instance_t>(this->address + offsets::instance::instancevalue::value);
    return ret;
}

void rbx::instance_t::SetBoolValue(bool rizz) {
    memory->write<bool>(this->address + rbx::offsets::instance::instancevalue::value, rizz);
}

rbx::instance_t rbx::instance_t::waitfor_child(std::string name, int timeout) {
    if (!this->address)
        return rbx::instance_t{};

    timeout *= 10;  // Converting timeout to intervals of 100 ms
    for (int times = 0; times < timeout; ++times) {
        auto child_list = memory->read<DWORD64>(this->address + offsets::instance::children);
        if (!child_list) continue; // Skip if child_list is invalid

        auto child_top = memory->read<DWORD64>(child_list);
        auto child_end = memory->read<DWORD64>(child_list + 0x8);

        for (DWORD64 child_addy = child_top; child_addy < child_end; child_addy += 0x10) {
            rbx::instance_t child = memory->read<rbx::instance_t>(child_addy);

            if (child.address > 1000 && child.getname() == name)
                return child;
        }
        Sleep(100); // Wait 100 ms before retrying
    }

    return rbx::instance_t{};  // Return an empty instance if child is not found within timeout
}


std::string rbx::instance_t::getname()
{
    const auto ptr = memory->read<std::uint64_t>(this->address + rbx::offsets::instance::name);

    if (ptr)
        return rbxstring(ptr);

    return "???";
}

std::string rbx::instance_t::getclassname()
{
    auto ClassDescriptor = memory->read<uint64_t>(address + rbx::offsets::instance::cdescriptor);
    auto ClassName = memory->read<uint64_t>(ClassDescriptor + rbx::offsets::instance::cname);

    if (ClassName)
        return rbxstring(ClassName);

    return "??? [ClassName]";
}

bool rbx::instance_t::IsA(std::string tocheck)
{
    if (getclassname() == tocheck) {
        return true;
    }

    return false;
}
