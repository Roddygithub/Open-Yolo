// 1. Inclure d'abord les en-têtes standards C++
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

// 2. Désactiver les avertissements de dépréciation pour GTK/GLib
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// 3. Inclure les en-têtes GTK/GTKmm (toujours avant X11)
#include <gdkmm.h>
#include <gtkmm.h>

// 4. Réactiver les avertissements
#pragma GCC diagnostic pop

// 5. Sauvegarder les macros X11 problématiques
#pragma push_macro("None")
#pragma push_macro("Bool")
#pragma push_macro("Status")
#pragma push_macro("True")
#pragma push_macro("False")

// 6. Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// 7. Inclure les en-têtes X11 dans un bloc extern "C"
extern "C" {
#undef None
#undef Bool
#undef Status
#undef True
#undef False
#define X_DISPLAY_MISSING
#include <X11/Xlib.h>
#include <X11/keysym.h>
}

// 8. Réactiver les avertissements
#pragma GCC diagnostic pop

// 9. Inclure les en-têtes du projet et autres bibliothèques
#include <nlohmann/json.hpp>

#include "input/KeyCodes.hpp"
#include "input/ShortcutConfig.hpp"

namespace input {

namespace {

const std::map<std::string, unsigned int> KEY_NAME_TO_CODE = {
    // Touches de contrôle
    {"Ctrl", Key::Ctrl},
    {"Alt", Key::Alt},
    {"Shift", Key::Shift},
    {"Super", Key::Super},

    // Touches de direction
    {"Up", Key::Up},
    {"Down", Key::Down},
    {"Left", Key::Left},
    {"Right", Key::Right},

    // Touches spéciales
    {"Space", Key::Space},
    {"Enter", Key::Enter},
    {"Escape", Key::Escape},
    {"Tab", Key::Tab},
    {"Backspace", Key::Backspace},

    // Touches de fonction
    {"F1", Key::F1},
    {"F2", Key::F2},
    {"F3", Key::F3},
    {"F4", Key::F4},
    {"F5", Key::F5},
    {"F6", Key::F6},
    {"F7", Key::F7},
    {"F8", Key::F8},
    {"F9", Key::F9},
    {"F10", Key::F10},
    {"F11", Key::F11},
    {"F12", Key::F12},

    // Chiffres
    {"0", Key::Num0},
    {"1", Key::Num1},
    {"2", Key::Num2},
    {"3", Key::Num3},
    {"4", Key::Num4},
    {"5", Key::Num5},
    {"6", Key::Num6},
    {"7", Key::Num7},
    {"8", Key::Num8},
    {"9", Key::Num9},

    // Lettres
    {"A", Key::A},
    {"B", Key::B},
    {"C", Key::C},
    {"D", Key::D},
    {"E", Key::E},
    {"F", Key::F},
    {"G", Key::G},
    {"H", Key::H},
    {"I", Key::I},
    {"J", Key::J},
    {"K", Key::K},
    {"L", Key::L},
    {"M", Key::M},
    {"N", Key::N},
    {"O", Key::O},
    {"P", Key::P},
    {"Q", Key::Q},
    {"R", Key::R},
    {"S", Key::S},
    {"T", Key::T},
    {"U", Key::U},
    {"V", Key::V},
    {"W", Key::W},
    {"X", Key::X},
    {"Y", Key::Y},
    {"Z", Key::Z},

    // Pavé numérique
    {"KP_Add", Key::KP_Add},
    {"KP_Subtract", Key::KP_Subtract},
    {"KP_Multiply", Key::KP_Multiply},
    {"KP_Divide", Key::KP_Divide},
    {"KP_Enter", Key::KP_Enter},
    {"KP_Decimal", Key::KP_Decimal},
    {"KP_0", Key::KP_0},
    {"KP_1", Key::KP_1},
    {"KP_2", Key::KP_2},
    {"KP_3", Key::KP_3},
    {"KP_4", Key::KP_4},
    {"KP_5", Key::KP_5},
    {"KP_6", Key::KP_6},
    {"KP_7", Key::KP_7},
    {"KP_8", Key::KP_8},
    {"KP_9", Key::KP_9},

    // Ponctuation
    {",", Key::Comma},
    {".", Key::Period},
    {";", Key::Semicolon},
    {"'", Key::Quote},
    {"[", Key::BracketLeft},
    {"]", Key::BracketRight},
    {"\\", Key::Backslash},
    {"-", Key::Minus},
    {"=", Key::Equal},
    {"`", Key::Grave}};

const std::map<unsigned int, std::string> KEY_CODE_TO_NAME = []() {
    std::map<unsigned int, std::string> result;
    for (const auto& [name, code] : KEY_NAME_TO_CODE) {
        result[code] = name;
    }
    return result;
}();

}  // namespace

ShortcutConfig::ShortcutConfig() { loadDefaultShortcuts(); }

bool ShortcutConfig::loadFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        file >> j;

        for (const auto& [action, keys] : j.items()) {
            if (keys.is_array()) {
                KeySequence sequence;
                for (const auto& key : keys) {
                    if (key.is_string()) {
                        std::string keyName = key.get<std::string>();
                        auto it = KEY_NAME_TO_CODE.find(keyName);
                        if (it != KEY_NAME_TO_CODE.end()) {
                            sequence.push_back(it->second);
                        }
                    }
                }
                if (!sequence.empty()) {
                    m_shortcuts[action] = sequence;
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading shortcuts config: " << e.what() << std::endl;
        return false;
    }
}

bool ShortcutConfig::saveToFile(const std::string& filepath) const {
    try {
        // Créer le répertoire parent s'il n'existe pas
        std::filesystem::path path(filepath);
        std::filesystem::create_directories(path.parent_path());

        nlohmann::json j;

        for (const auto& [action, keys] : m_shortcuts) {
            nlohmann::json keyArray = nlohmann::json::array();
            for (const auto& key : keys) {
                auto it = KEY_CODE_TO_NAME.find(key);
                if (it != KEY_CODE_TO_NAME.end()) {
                    keyArray.push_back(it->second);
                }
            }
            if (!keyArray.empty()) {
                j[action] = keyArray;
            }
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return false;
        }

        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving shortcuts config: " << e.what() << std::endl;
        return false;
    }
}

void ShortcutConfig::setShortcut(const std::string& action, const KeySequence& keys) {
    if (!keys.empty()) {
        m_shortcuts[action] = keys;
    } else {
        m_shortcuts.erase(action);
    }
}

const ShortcutConfig::KeySequence& ShortcutConfig::getShortcut(const std::string& action) const {
    static const KeySequence empty;
    auto it = m_shortcuts.find(action);
    return it != m_shortcuts.end() ? it->second : empty;
}

const ShortcutConfig::ShortcutMap& ShortcutConfig::getAllShortcuts() const { return m_shortcuts; }

void ShortcutConfig::resetToDefaults() {
    m_shortcuts.clear();
    loadDefaultShortcuts();
}

std::string ShortcutConfig::keySequenceToString(const KeySequence& keys) {
    std::stringstream ss;
    for (size_t i = 0; i < keys.size(); ++i) {
        auto it = KEY_CODE_TO_NAME.find(keys[i]);
        if (it != KEY_CODE_TO_NAME.end()) {
            if (i > 0)
                ss << "+";
            ss << it->second;
        }
    }
    return ss.str();
}

ShortcutConfig::KeySequence ShortcutConfig::stringToKeySequence(const std::string& str) {
    KeySequence result;
    std::stringstream ss(str);
    std::string keyName;

    while (std::getline(ss, keyName, '+')) {
        // Supprimer les espaces
        keyName.erase(std::remove_if(keyName.begin(), keyName.end(), ::isspace), keyName.end());

        if (!keyName.empty()) {
            // Essayer de trouver la touche avec la casse exacte
            auto it = KEY_NAME_TO_CODE.find(keyName);
            if (it == KEY_NAME_TO_CODE.end()) {
                // Essayer en majuscules si non trouvé
                std::transform(keyName.begin(), keyName.end(), keyName.begin(), ::toupper);
                it = KEY_NAME_TO_CODE.find(keyName);
            }

            if (it != KEY_NAME_TO_CODE.end()) {
                result.push_back(it->second);
            }
        }
    }

    return result;
}

void ShortcutConfig::loadDefaultShortcuts() {
    m_shortcuts = {{"toggle_visible", {Key::Ctrl, Key::Alt, Key::S}},
                   {"increase_size", {Key::Ctrl, Key::Alt, Key::Up}},
                   {"decrease_size", {Key::Ctrl, Key::Alt, Key::Down}},
                   {"next_theme", {Key::Ctrl, Key::Alt, Key::Right}},
                   {"prev_theme", {Key::Ctrl, Key::Alt, Key::Left}},
                   {"reset_cursor", {Key::Ctrl, Key::Alt, Key::R}},
                   {"toggle_tracking", {Key::Ctrl, Key::Alt, Key::P}},
                   {"toggle_lock", {Key::Ctrl, Key::Alt, Key::L}},
                   {"toggle_game_mode", {Key::Ctrl, Key::Alt, Key::G}},
                   {"increase_opacity", {Key::Ctrl, Key::Alt, Key::KP_Add}},
                   {"decrease_opacity", {Key::Ctrl, Key::Alt, Key::KP_Subtract}},
                   {"set_theme_1", {Key::Ctrl, Key::Alt, Key::Num1}},
                   {"set_theme_2", {Key::Ctrl, Key::Alt, Key::Num2}},
                   {"set_theme_3", {Key::Ctrl, Key::Alt, Key::Num3}},
                   {"set_theme_4", {Key::Ctrl, Key::Alt, Key::Num4}},
                   {"set_theme_5", {Key::Ctrl, Key::Alt, Key::Num5}},
                   {"set_theme_6", {Key::Ctrl, Key::Alt, Key::Num6}},
                   {"set_theme_7", {Key::Ctrl, Key::Alt, Key::Num7}},
                   {"set_theme_8", {Key::Ctrl, Key::Alt, Key::Num8}},
                   {"set_theme_9", {Key::Ctrl, Key::Alt, Key::Num9}}};
}

// 10. Restaurer les définitions de macros X11
#pragma pop_macro("False")
#pragma pop_macro("True")
#pragma pop_macro("Status")
#pragma pop_macro("Bool")
#pragma pop_macro("None")

}  // namespace input
