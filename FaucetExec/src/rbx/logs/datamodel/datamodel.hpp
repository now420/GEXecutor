#pragma once
#include <fstream>
#include <thread>
#include <cstdlib>
#include <Windows.h>
#include <ShlObj.h>
#include <vector>
#include <filesystem>
#include <string>
#include <algorithm>
#include <cstdint>

namespace booty {

    static std::wstring get_appdata_path();

    std::vector<std::filesystem::path> get_roblox_file_logs();

    std::filesystem::path GetLatestLog();

    std::uint64_t GetRenderView();

    std::uint64_t get_game_id();

}
