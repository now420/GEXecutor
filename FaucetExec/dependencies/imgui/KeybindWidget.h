#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <string>
#include <unordered_set>
#include <vector>
#include <windows.h>
#include <chrono>
#include <unordered_map>

const std::unordered_map<int, std::string> keyNames = {
    {0x01, "MB1"},
    {0x02, "MB2"},
    {0x03, "CTRL_BRK"},
    {0x04, "MMB"},
    {0x05, "X1"},
    {0x06, "X2"},
    {0x08, "BKSP"},
    {0x09, "Tab"},
    {0x0C, "CLR"},
    {0x0D, "Enter"},
    {0x10, "Shift"},
    {0x11, ""},
    {0x12, "Alt"},
    {0x13, "Pause"},
    {0x14, "Caps Lock"},
    {0x1B, "ESC"},
    {0x20, "Space"},
    {0x21, "Page Up"},
    {0x22, "Page Down"},
    {0x23, "End"},
    {0x24, "Home"},
    {0x25, "Left"},
    {0x26, "Up"},
    {0x27, "Right"},
    {0x28, "Down"},
    {0x29, "Select"},
    {0x2A, "Print"},
    {0x2B, "Execute"},
    {0x2C, "Print Screen"},
    {0x2D, "Insert"},
    {0x2E, "Delete"},
    {0x2F, "Help"},
    {0x30, "0"},
    {0x31, "1"},
    {0x32, "2"},
    {0x33, "3"},
    {0x34, "4"},
    {0x35, "5"},
    {0x36, "6"},
    {0x37, "7"},
    {0x38, "8"},
    {0x39, "9"},
    {0x41, "A"},
    {0x42, "B"},
    {0x43, "C"},
    {0x44, "D"},
    {0x45, "E"},
    {0x46, "F"},
    {0x47, "G"},
    {0x48, "H"},
    {0x49, "I"},
    {0x4A, "J"},
    {0x4B, "K"},
    {0x4C, "L"},
    {0x4D, "M"},
    {0x4E, "N"},
    {0x4F, "O"},
    {0x50, "P"},
    {0x51, "Q"},
    {0x52, "R"},
    {0x53, "S"},
    {0x54, "T"},
    {0x55, "U"},
    {0x56, "V"},
    {0x57, "W"},
    {0x58, "X"},
    {0x59, "Y"},
    {0x5A, "Z"},
    {0x5B, "Left Windows"},
    {0x5C, "Right Windows"},
    {0x5D, "Applications"},
    {0x5F, "Sleep"},
    {0x60, "Numpad 0"},
    {0x61, "Numpad 1"},
    {0x62, "Numpad 2"},
    {0x63, "Numpad 3"},
    {0x64, "Numpad 4"},
    {0x65, "Numpad 5"},
    {0x66, "Numpad 6"},
    {0x67, "Numpad 7"},
    {0x68, "Numpad 8"},
    {0x69, "Numpad 9"},
    {0x6A, "Multiply"},
    {0x6B, "Add"},
    {0x6C, "Separator"},
    {0x6D, "Subtract"},
    {0x6E, "Decimal"},
    {0x6F, "Divide"},
    {0x70, "F1"},
    {0x71, "F2"},
    {0x72, "F3"},
    {0x73, "F4"},
    {0x74, "F5"},
    {0x75, "F6"},
    {0x76, "F7"},
    {0x77, "F8"},
    {0x78, "F9"},
    {0x79, "F10"},
    {0x7A, "F11"},
    {0x7B, "F12"},
    {0x7C, "F13"},
    {0x7D, "F14"},
    {0x7E, "F15"},
    {0x7F, "F16"},
    {0x80, "F17"},
    {0x81, "F18"},
    {0x82, "F19"},
    {0x83, "F20"},
    {0x84, "F21"},
    {0x85, "F22"},
    {0x86, "F23"},
    {0x87, "F24"},
    {0x90, "Num Lock"},
    {0x91, "Scroll Lock"},
    {0xA0, "LShift"},
    {0xA1, "RShift"},
    {0xA2, "LCTRL"},
    {0xA3, "RCTRL"},
    {0xA4, "L_Alt"},
    {0xA5, "R_Alt"},
    {0xA6, "Browser Back"},
    {0xA7, "Browser Forward"},
    {0xA8, "Browser Refresh"},
    {0xA9, "Browser Stop"},
    {0xAA, "Browser Search"},
    {0xAB, "Browser Favorites"},
    {0xAC, "Browser Home"},
    {0xAD, "Volume Mute"},
    {0xAE, "Volume Down"},
    {0xAF, "Volume Up"},
    {0xB0, "Next Track"},
    {0xB1, "Previous Track"},
    {0xB2, "Stop Media"},
    {0xB3, "Play/Pause"},
    {0xB4, "Launch Mail"},
    {0xB5, "Launch Media Select"},
    {0xB6, "Launch App 1"},
    {0xB7, "Launch App 2"},
    {0xBA, "Semicolon (;)"},
    {0xBB, "Equal Sign (=)"},
    {0xBC, "Comma (,)"},
    {0xBD, "Minus (-)"},
    {0xBE, "Period (.)"},
    {0xBF, "Forward Slash (/)"},
    {0xC0, "Grave Accent (`)"},
    {0xDB, "Open Bracket ([)"},
    {0xDC, "Backslash (\\)"},
    {0xDD, "Close Bracket (])"},
    {0xDE, "Single Quote (')"},
    {0xE2, "OEM 102"},
    {0xE5, "IME Process"},
    {0xE7, "Packet"},
    {0xF6, "Attn"},
    {0xF7, "CrSel"},
    {0xF8, "ExSel"},
    {0xF9, "Erase EOF"},
    {0xFA, "Play"},
    {0xFB, "Zoom"},
    {0xFE, "PA1"},
    {0xFF, "Clear"}
};

namespace bootye {
    inline std::string GetKeyNamere(int keycode) {
        auto it = keyNames.find(keycode);
        if (it != keyNames.end()) {
            return it->second;
        }
        return "Unknown";
    }

    inline std::string GetKeyCombinationName(const std::unordered_set<int>& keyCombination) {
        std::string result;
        for (int keycode : keyCombination) {
            if (!result.empty()) {
                result += " + ";
            }
            result += GetKeyNamere(keycode);
        }
        return result.empty() ? "[None]" : result;
    }
}

struct KeybindWidget {
    mutable std::chrono::steady_clock::time_point lastCheckTime = std::chrono::steady_clock::now();
    std::string label;  // Keybind label
    std::unordered_set<int> keyCombination;  // Stores key combination
    bool waiting_for_input; // Flag to check if we are waiting for input
    bool enabled;       // Property to enable/disable the widget
    bool capture_complete; // Flag to indicate if input capture is done
    enum Mode { Hold, Toggle };  // Mode for keybind (Hold or Toggle)
    Mode currentMode;  // Stores the current mode (Hold/Toggle)

    KeybindWidget(const std::string& label)
        : label(label), waiting_for_input(false), enabled(true), capture_complete(false), currentMode(Hold) {}

    void Render() {
        if (!enabled) return;

        std::string keyCombinationStr = waiting_for_input ? "[Press keys]" : bootye::GetKeyCombinationName(keyCombination);

        // Show button with the key combination or prompt to press keys
        if (ImGui::Button((label + ": " + keyCombinationStr).c_str())) {
            waiting_for_input = true;
            keyCombination.clear();
            capture_complete = false; // Reset the capture status when starting input
        }

        // Right-click brings up the mode selection menu (Hold/Toggle)
        if (ImGui::IsItemClicked(1)) {  // 1 = Right-click
            ImGui::OpenPopup("Keybind Mode");
        }

        // Display the popup to choose between Hold and Toggle modes
        if (ImGui::BeginPopup("Keybind Mode")) {
            if (ImGui::Selectable("Hold", currentMode == Hold)) {
                currentMode = Hold;
            }
            if (ImGui::Selectable("Toggle", currentMode == Toggle)) {
                currentMode = Toggle;
            }
            ImGui::EndPopup();
        }

        // While waiting for input, capture the keys
        if (waiting_for_input && !capture_complete) {
            bool anyKeyPressed = false;

            for (int i = 0x01; i <= 0xFE; ++i) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    keyCombination.insert(i);  // Add pressed key to combination
                    anyKeyPressed = true;      // At least one key is being pressed
                }
            }

            // Only stop waiting for input when all keys are released
            if (!anyKeyPressed && !keyCombination.empty()) {
                waiting_for_input = false;
                capture_complete = true; // Mark the input as captured
            }
        }
    }

    bool CheckEnabled() const {
        const auto now = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheckTime);

        static std::unordered_map<int, bool> keyStateLastFrame;
        static int debounceCounter = 0;  // Debounce counter for key state

            lastCheckTime = now;

            if (enabled && !keyCombination.empty()) {
                if (currentMode == Hold) {
                    bool allKeysPressed = true;
                    for (const int keycode : keyCombination) {
                        if (keycode < 0x01 || keycode > 0xFF) {
                            return false;
                        }

                        if (keyCombination.empty()) {
                            return false;
                        }

                        if (keyCombination.size() == 0) {
                            return false;
                        }

                        const short keyState = GetAsyncKeyState(keycode);
                        if ((keyState & 0x8000) == 0) {
                            allKeysPressed = false; 
                            break;
                        }
                    }

                    if (allKeysPressed) {
                        debounceCounter = 0;  
                        return true;
                    }
                    else {
                        if (debounceCounter < 5) {
                            debounceCounter++;
                            return true;
                        }
                        return false;
                    }
                }
                else if (currentMode == Toggle) {
                    static bool toggledState = false;

                    for (const int keycode : keyCombination) {
                        if (keycode < 0x01 || keycode > 0xFF) {
                            return false;
                        }
                        const short keyState = GetAsyncKeyState(keycode);

                        if ((keyState & 0x8000) && !keyStateLastFrame[keycode]) {
                            toggledState = !toggledState;
                            break;
                        }

                        keyStateLastFrame[keycode] = (keyState & 0x8000) != 0;
                    }

                    return toggledState;
                }
            }
        return false;
    }
};
