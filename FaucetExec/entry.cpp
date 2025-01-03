#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "src/rbx/storage/storage.hpp"
#include "src/rbx/logs/datamodel/datamodel.hpp"
#include "src/utils/memory/memory.hpp"
#include "src/rbx/instance/instance.hpp"
#include <tlhelp32.h>
#include <windows.h>
#include <psapi.h>
#include "dependencies/streamio/streamio.hpp"
#include "dependencies/xxhash.h"
#include "dependencies/zstd/zstd.h"
#include "dependencies/Luau/Compiler.h"
#include "dependencies/Luau/BytecodeBuilder.h"
#include "dependencies/Luau/BytecodeUtils.h"
#include "src/overlay/overlay.hpp"

#pragma comment(lib, "ws2_32.lib")

DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe)) {
            do {
                if (std::wstring(pe.szExeFile) == processName) {
                    processId = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return processId;
}

std::vector<char> compress_jest(const std::string& bytecode, size_t& byte_size) {
    const auto data_size = bytecode.size();
    const auto max_size = ZSTD_compressBound(data_size);
    auto buffer = std::vector<char>(max_size + 8);

    // "RSB1" prefix and data size
    buffer[0] = 'R'; buffer[1] = 'S'; buffer[2] = 'B'; buffer[3] = '1';
    std::memcpy(&buffer[4], &data_size, sizeof(data_size));

    const auto compressed_size = ZSTD_compress(&buffer[8], max_size, bytecode.data(), data_size, ZSTD_maxCLevel());
    if (ZSTD_isError(compressed_size)) {
        throw std::runtime_error("Failed to compress the bytecode.");
    }

    const auto size = compressed_size + 8;
    const auto key = XXH32(buffer.data(), size, 42u);
    const auto bytes = reinterpret_cast<const uint8_t*>(&key);

    // XOR encryption loop
    for (auto i = 0u; i < size; ++i) {
        buffer[i] ^= bytes[i % 4] + i * 41u;
    }

    byte_size = size;
    buffer.resize(size);  // Shrink buffer to actual size

    return buffer;
}

class bytecode_encoder_t : public Luau::BytecodeEncoder {
    inline void encode(uint32_t* data, size_t count) override {
        for (size_t i = 0; i < count;) {
            auto& opcode = *reinterpret_cast<uint8_t*>(data + i);
            i += Luau::getOpLength(LuauOpcode(opcode));
            opcode *= 227;
        }
    }
};

Luau::CompileOptions compile_options;

std::string compile(const std::string& source) {
    if (compile_options.debugLevel != 2 || compile_options.optimizationLevel != 2) {
        compile_options.debugLevel = 2;
        compile_options.optimizationLevel = 2;
    }

    static bytecode_encoder_t encoder;

    // Compilation with larger data handling
    std::string bytecode = Luau::compile(source, {}, {}, &encoder);
    return bytecode;
}

HWND g_HWND = NULL;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    if (windowProcessId == (DWORD)lParam) {
        g_HWND = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND getWindowHandleFromProcessId(DWORD processId) {
    EnumWindows(EnumWindowsProc, processId);
    return g_HWND;
}


std::string GetLuaScript() {
    HRSRC resourceHandle = FindResourceW(NULL, MAKEINTRESOURCEW(101), RT_RCDATA);
    if (resourceHandle == NULL) return "";

    HGLOBAL loadedResource = LoadResource(NULL, resourceHandle);
    if (loadedResource == NULL) return "";

    DWORD size = SizeofResource(NULL, resourceHandle);
    void* data = LockResource(loadedResource);

    return std::string(static_cast<char*>(data), size);
}


int main()
{
    std::cout << "\033[32m[GEX INJECTOR]\033[0m Finding Roblox!" << std::endl;
    std::string luaContent = GetLuaScript();

    DWORD pid = GetProcessIdByName(L"RobloxPlayerBeta.exe");

    if (pid == 0) {
        std::cerr << "\033[32m[GEX INJECTOR]\033[0mOpen Roblox Player!" << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "\033[32m[GEX INJECTOR]\033[0m Roblox Found!" << std::endl;
    std::cout << "\033[32m[GEX INJECTOR]\033[0m Injecting!" << std::endl;
    memory->attach(pid);
    auto window = getWindowHandleFromProcessId(pid);

    auto renderview = booty::GetRenderView();

    auto dmptr = memory->read<uintptr_t>(renderview + 0x118);

    auto dmaddy = memory->read<uintptr_t>(dmptr + 0x1a8);

    storage::datamodel = static_cast<rbx::instance_t>(dmaddy);

    rbx::instance_t corePackages = storage::datamodel.findfirstchild("CorePackages");
    rbx::instance_t CoreGui = storage::datamodel.findfirstchild("CoreGui");
    rbx::instance_t RobloxGui = CoreGui.findfirstchild("RobloxGui");
    rbx::instance_t Modules = RobloxGui.findfirstchild("Modules");

    // things for jg
    rbx::instance_t corepkgs = storage::datamodel.findfirstchild("CorePackages");;

    rbx::instance_t ws = corepkgs.findfirstchild("Workspace");

    rbx::instance_t pkgs = ws.findfirstchild("Packages");

    rbx::instance_t _ws = pkgs.findfirstchild("_Workspace");

    rbx::instance_t smsp = _ws.findfirstchild("SMSProtocol");

    rbx::instance_t dev = smsp.findfirstchild("Dev");

    rbx::instance_t jestglobals = dev.findfirstchild("JestGlobals");

    rbx::instance_t player_list = Modules.findfirstchild("PlayerList");
    rbx::instance_t player_list_manager = player_list.findfirstchild("PlayerListManager");

    rbx::instance_t Common = Modules.findfirstchild("Common");
    rbx::instance_t Url = Common.findfirstchild("Url");

    jestglobals.modulebypassi();

    size_t old_bytecode_size;
    std::vector<char> old_bytecode;

    // gets current jestglobals bytecode

    jestglobals.GetBytecode(old_bytecode, old_bytecode_size);

    // hijack
    memory->write<uintptr_t>(player_list_manager.address + 0x8, jestglobals.address);
    
    size_t target_bytecode_size;
    auto raper = compress_jest(compile(luaContent), target_bytecode_size);

    // sets the bytecode

    jestglobals.SetBytecode(raper, target_bytecode_size);

    // waits for init to be created

    //jestglobals.waitfor_child("Initialized", 5); COMMENTED OUT DUE TO A FUCKIN POINTER ISSUE

    // this sends an escape, it fires JestGlobal
    HWND CurrentWindow = GetForegroundWindow();
    SendMessage(window, WM_CLOSE, NULL, NULL);

    // apply the old bytecode, and return player_list_managers address to itself.
    jestglobals.SetBytecode(old_bytecode, old_bytecode_size);
    memory->write<uintptr_t>(player_list_manager.address + 0x8, player_list_manager.address);

    //Url.SetBytecode(rapist, sizeof(raper.size()));
    std::cout << "\033[32m[GEX INJECTOR]\033[0m Successfully injected!" << std::endl;
    storage::jestglobals = jestglobals;
    std::cout << "\033[31m[IMPORTANT]\033[0m Do not close this console!" << std::endl;

    std::thread(gui::overlay::render).detach();
    
    std::cout << "\033[34m[INFO]\033[0m Hit END or F10 on your keyboard to toggle the UI!" << std::endl;
    for (int i = 0; i++; i < 100) {
    }

    while (true) {}
}
//better looking injector by diet cock fr 
