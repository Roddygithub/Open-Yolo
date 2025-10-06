// 1. Inclure d'abord les en-têtes standards C++
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

// 2. Désactiver les avertissements de dépréciation pour GTK/GLib
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// 3. Inclure les en-têtes GTK/GTKmm (toujours avant X11)
#include <gdkmm.h>
#include <gtkmm.h>

// 4. Réactiver les avertissements
#pragma GCC diagnostic pop

// 5. Définir _XOPEN_SOURCE pour X11
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

// 6. Sauvegarder les macros X11 problématiques
#pragma push_macro("None")
#pragma push_macro("Bool")
#pragma push_macro("Status")
#pragma push_macro("True")
#pragma push_macro("False")

// 7. Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// 8. Inclure les en-têtes X11 dans un bloc extern "C"
extern "C" {
    #undef None
    #undef Bool
    #undef Status
    #undef True
    #undef False
    #define X_DISPLAY_MISSING
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/XKBlib.h>
    #include <X11/extensions/XInput2.h>
}

// 9. Définir des types compatibles
using XBool = int;
using XStatus = int;

// 10. Réactiver les avertissements
#pragma GCC diagnostic pop

// 11. En-têtes du projet
#include "input/ShortcutManager.hpp"

namespace input {

ShortcutManager::ShortcutManager() : m_display(nullptr), xi_opcode(-1) {
    // Définir le chemin par défaut pour la configuration
    const char* home = getenv("HOME");
    if (home) {
        m_configPath = std::string(home) + "/.config/open-yolo/shortcuts.json";
    } else {
        m_configPath = "/tmp/open-yolo-shortcuts.json";
    }
}

ShortcutManager::~ShortcutManager() {
    if (m_display) {
        XCloseDisplay(m_display);
    }
}

bool ShortcutManager::initialize() {
    // Ouvrir la connexion au serveur X
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        std::cerr << "Failed to open X display" << std::endl;
        return false;
    }
    
    // Vérifier l'extension XInput2
    int event, error;
    if (!XQueryExtension(m_display, "XInputExtension", &xi_opcode, &event, &error)) {
        std::cerr << "X Input extension not available" << std::endl;
        return false;
    }
    
    // Activer les événements clavier XInput2
    XIEventMask event_mask;
    unsigned char mask[4] = {0}; // Pour stocker le masque de bits
    
    event_mask.deviceid = XIAllMasterDevices;
    event_mask.mask_len = sizeof(mask);
    event_mask.mask = mask;
    
    // Activer les événements de touches pressées et relâchées
    XISetMask(mask, XI_KeyPress);
    XISetMask(mask, XI_KeyRelease);
    
    XISelectEvents(m_display, DefaultRootWindow(m_display), &event_mask, 1);
    XSync(m_display, false);
    
    // Initialiser les états des touches
    m_keyStates.resize(256, false);
    
    // Charger la configuration
    loadConfig();
    
    return true;
}

void ShortcutManager::update() {
    if (!m_display) return;
    
    // Traiter les événements en attente
    int num_events = XPending(m_display);
    
    for (int i = 0; i < num_events; ++i) {
        XEvent event;
        XNextEvent(m_display, &event);
        
        // Vérifier si c'est un événement XInput2
        if (event.xcookie.type == GenericEvent && 
            event.xcookie.extension == xi_opcode) {
                
            if (XGetEventData(m_display, &event.xcookie)) {
                // Traiter les événements de touche
                if (event.xcookie.evtype == XI_KeyPress || 
                    event.xcookie.evtype == XI_KeyRelease) {
                    
                    XIDeviceEvent* xiev = (XIDeviceEvent*)event.xcookie.data;
                    KeySym keysym = XkbKeycodeToKeysym(m_display, xiev->detail, 0, 0);
                    
                    // Mettre à jour l'état de la touche
                    if (event.xcookie.evtype == XI_KeyPress) {
                        m_keyStates[keysym & 0xFF] = true;
                    } else {
                        m_keyStates[keysym & 0xFF] = false;
                    }
                }
                XFreeEventData(m_display, &event.xcookie);
            }
        }
    }
    
    // Vérifier les raccourcis
    for (auto& pair : m_shortcuts) {
        Shortcut& shortcut = pair.second;
        if (!shortcut.isEnabled) continue;
        
        bool allKeysPressed = true;
        bool anyKeyChanged = false;
        
        // Vérifier si toutes les touches du raccourci sont pressées
        for (unsigned int key : shortcut.keys) {
            KeyCode keycode = XKeysymToKeycode(m_display, key);
            if (keycode >= m_keyStates.size() || !m_keyStates[keycode]) {
                allKeysPressed = false;
                break;
            }
            
            // Vérifier si une touche vient d'être pressée
            if (m_keyStates[keycode] != shortcut.wasPressed) {
                anyKeyChanged = true;
            }
        }
        
        // Si toutes les touches sont pressées et qu'au moins une vient de changer
        if (allKeysPressed && anyKeyChanged) {
            // Déclencher le callback uniquement sur le front montant
            if (!shortcut.wasPressed && shortcut.callback) {
                shortcut.callback();
            }
        }
        
        shortcut.wasPressed = allKeysPressed;
    }
}

void ShortcutManager::registerShortcut(const std::vector<unsigned int>& keys, Callback callback, const std::string& action) {
    if (keys.empty()) return;
    
    // Vérifier si une action avec le même nom existe déjà
    if (!action.empty()) {
        auto it = m_actionToKeys.find(action);
        if (it != m_actionToKeys.end()) {
            // Supprimer l'ancien raccourci
            m_shortcuts.erase(it->second);
        }
        m_actionToKeys[action] = keys;
    }
    
    m_shortcuts[keys] = {keys, callback, action};
}

void ShortcutManager::unregisterShortcut(const std::vector<unsigned int>& keys) {
    auto it = m_shortcuts.find(keys);
    if (it != m_shortcuts.end()) {
        if (!it->second.action.empty()) {
            m_actionToKeys.erase(it->second.action);
        }
        m_shortcuts.erase(it);
    }
}

void ShortcutManager::unregisterAction(const std::string& action) {
    auto it = m_actionToKeys.find(action);
    if (it != m_actionToKeys.end()) {
        m_shortcuts.erase(it->second);
        m_actionToKeys.erase(it);
    }
}

void ShortcutManager::loadConfig(const std::string& configPath) {
    std::string path = configPath.empty() ? m_configPath : configPath;
    
    if (!m_config.loadFromFile(path)) {
        std::cerr << "Failed to load shortcuts config, using defaults" << std::endl;
        m_config.resetToDefaults();
        saveConfig(path);
    }
}

void ShortcutManager::saveConfig(const std::string& configPath) {
    std::string path = configPath.empty() ? m_configPath : configPath;
    if (!m_config.saveToFile(path)) {
        std::cerr << "Failed to save shortcuts config to " << path << std::endl;
    }
}

void ShortcutManager::resetToDefaultConfig() {
    m_config.resetToDefaults();
    saveConfig();
}

ShortcutConfig& ShortcutManager::getConfig() {
    return m_config;
}

const ShortcutConfig& ShortcutManager::getConfig() const {
    return m_config;
}

bool ShortcutManager::checkShortcut(const Shortcut& shortcut) {
    if (shortcut.keys.empty()) return false;
    
    for (auto key : shortcut.keys) {
        KeyCode keycode = XKeysymToKeycode(m_display, key);
        if (keycode == 0) continue;
        
        if (keycode >= m_keyStates.size() || !m_keyStates[keycode]) {
            return false;
        }
    }
    
    return true;
}

void ShortcutManager::updateKeyStates() {
    if (!m_display) return;
    
    char keys[32];
    XQueryKeymap(m_display, keys);
    
    // Réinitialiser tous les états à false
    std::fill(m_keyStates.begin(), m_keyStates.end(), false);
    
    // Mettre à jour les états des touches actuellement pressées
    for (int i = 0; i < 256; ++i) {
        if (keys[i / 8] & (1 << (i % 8))) {
            KeySym keysym = XkbKeycodeToKeysym(m_display, i, 0, 0);
            if (keysym != NoSymbol) {
                m_keyStates[keysym & 0xFF] = true;
            }
        }
    }
}

// 12. Restaurer les définitions de macros X11
#pragma pop_macro("False")
#pragma pop_macro("True")
#pragma pop_macro("Status")
#pragma pop_macro("Bool")
#pragma pop_macro("None")

} // namespace input
