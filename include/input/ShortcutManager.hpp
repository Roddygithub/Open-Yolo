#pragma once

// Inclusions standards
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <string>

// Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Définir X_DISPLAY_MISSING avant d'inclure Xlib.h
#define X_DISPLAY_MISSING

// Inclure les en-têtes X11
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Réactiver les avertissements
#pragma GCC diagnostic pop

// Définir des types compatibles
using XBool = int;
using XStatus = int;

// En-tête du projet
#include "input/ShortcutConfig.hpp"

namespace input {

class ShortcutManager {
public:
    using Callback = std::function<void()>;
    
    ShortcutManager();
    ~ShortcutManager();
    
    // Initialisation et mise à jour
    bool initialize();
    void update();
    
    // Gestion des raccourcis
    void registerShortcut(const std::vector<unsigned int>& keys, Callback callback, const std::string& action = "");
    void unregisterShortcut(const std::vector<unsigned int>& keys);
    void unregisterAction(const std::string& action);
    
    // Gestion de la configuration
    void loadConfig(const std::string& configPath = "");
    void saveConfig(const std::string& configPath = "");
    void resetToDefaultConfig();
    
    // Accès à la configuration
    ShortcutConfig& getConfig();
    const ShortcutConfig& getConfig() const;

private:
    struct Shortcut {
        std::vector<unsigned int> keys;
        Callback callback;
        std::string action;
        bool wasPressed = false;
        bool isEnabled = true;
    };

    Display* m_display;
    int xi_opcode; // Extension XInput
    std::map<std::vector<unsigned int>, Shortcut> m_shortcuts;
    std::map<std::string, std::vector<unsigned int>> m_actionToKeys;
    std::vector<bool> m_keyStates;
    ShortcutConfig m_config;
    std::string m_configPath;
    
    // Méthodes internes
    bool checkShortcut(const Shortcut& shortcut);
    void updateKeyStates();
};

} // namespace input
