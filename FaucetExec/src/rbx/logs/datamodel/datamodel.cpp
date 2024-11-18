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
#include <iostream>
#include "datamodel.hpp"

namespace booty {

    static std::wstring get_appdata_path() {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
            return std::wstring(path);
        }
        return L"";
    }

    std::vector<std::filesystem::path> get_roblox_file_logs() {
        std::vector<std::filesystem::path> roblox_logs;
        std::wstring roblox_log_path = get_appdata_path() + L"\\Roblox\\logs";

        try {
            for (const auto& entry : std::filesystem::directory_iterator(roblox_log_path)) {
                if (entry.is_regular_file() &&
                    entry.path().extension() == L".log" &&
                    entry.path().filename().wstring().find(L"Player") != std::wstring::npos) {
                    roblox_logs.push_back(entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cout << "NIGGER ERROr" << std::endl;
        }

        return roblox_logs;
    }

    std::filesystem::path GetLatestLog() {
        auto logs = get_roblox_file_logs();
        if (logs.empty()) {
            throw std::runtime_error("No Roblox log files found.");
        }

        std::sort(logs.begin(), logs.end(), [](const auto& a, const auto& b) {
            return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
            });

        return logs.front();
    }

    std::uint64_t GetRenderView() {
        auto latest_log = GetLatestLog();
        std::ifstream nigger(latest_log);

        if (!nigger.is_open()) {
            throw std::runtime_error("Unable to open Roblox log file.");
        }

        std::string log_line;
        while (std::getline(nigger, log_line)) {
            size_t start_pos = log_line.find("initialize view(");
            if (start_pos != std::string::npos) {
                start_pos += std::string("initialize view(").length();
                size_t end_pos = log_line.find(')', start_pos);
                if (end_pos != std::string::npos) {
                    std::string view_hex_str = log_line.substr(start_pos, end_pos - start_pos);
                    std::uint64_t render_view = std::strtoull(view_hex_str.c_str(), nullptr, 16);
                    return render_view;
                }
            }
        }

        throw std::runtime_error("Render view not found in the log file.");
    }

    std::uint64_t get_game_id() {
        auto latest_log = GetLatestLog();
        std::ifstream rbx_log(latest_log);

        if (!rbx_log.is_open()) {
            throw std::runtime_error("Unable to open Roblox log file.");
        }

        std::string log_line;
        std::string game_id_str;
        while (std::getline(rbx_log, log_line)) {
            size_t start_pos = log_line.find("[FLog::Output] ! Joining game");
            if (start_pos != std::string::npos) {
                start_pos = log_line.find("place ", start_pos);
                if (start_pos != std::string::npos) {
                    start_pos += std::string("place ").length();
                    size_t end_pos = log_line.find(' ', start_pos);
                    if (end_pos != std::string::npos) {
                        game_id_str = log_line.substr(start_pos, end_pos - start_pos);
                    }
                }
            }
        }

        if (!game_id_str.empty()) {
            std::uint64_t game_id = std::strtoull(game_id_str.c_str(), nullptr, 10);
            return game_id;
        }

        throw std::runtime_error("Game ID not found in the log file.");
    }

}
