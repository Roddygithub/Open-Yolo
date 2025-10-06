#pragma once

// Standard C++
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>
#include <gtkmm.h>
#include <gdk/gdkx.h>

// Définition de la macro LOG_ERROR si non définie
#ifndef LOG_ERROR
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl;
#endif

// Déclaration anticipée
namespace cursor_manager {
    class CursorManager;
}

namespace input {
    class X11Backend;
}

// Project headers - Inclure d'abord les en-têtes du projet
#include "../InputBackend.hpp"
#include "../CursorError.hpp"
#include "../../cursormanager/CursorManager.hpp"

// Sauvegarder les définitions de macros problématiques
#pragma push_macro("None")
#pragma push_macro("Bool")
#pragma push_macro("Status")
#pragma push_macro("True")
#pragma push_macro("False")

// Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Définir X_DISPLAY_MISSING avant d'inclure Xlib.h
#define X_DISPLAY_MISSING

// Inclure les en-têtes X11 de base
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

// Définir des types compatibles
using XBool = int;
using XStatus = int;

// Réactiver les avertissements
#pragma GCC diagnostic pop

// Inclure les extensions X11 avec gestion des avertissements
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <X11/extensions/Xfixes.h>
#include <X11/Xcursor/Xcursor.h>
#pragma GCC diagnostic pop

// Alias pour le type de curseur
using CursorType = cursor_manager::CursorManager::CursorType;

class X11Backend : public InputBackend {
public:
    explicit X11Backend(Display* display);
    ~X11Backend() override;
    
    // Implémentation de l'interface InputBackend
    bool isInitialized() const override { return m_initialized; }
    bool initialize() override;
    bool isAvailable() const override;
    void registerShortcut(const std::string& name, const std::string& accelerator, KeyCallback callback) override;
    void unregisterShortcut(const std::string& name) override;
    bool handleKeyEvent(guint keyval, GdkModifierType mods) override;
    bool onKeyPressed(GdkEventKey* event);
    // Méthodes spécifiques à X11
    bool setCursor(CursorType type,
                  const std::string& cursorPath = "",
                  int size = 32,
                  int hotspotY = 0);

    // Active/désactive l'utilisation des curseurs système
    void useSystemCursors(bool useSystem = true);
    
    // Met à jour la position du curseur
    void setCursorPosition(int x, int y);
    
    // Récupère la position actuelle du curseur
    std::pair<int, int> getCursorPosition() const;
    
    // Active/désactive la visibilité du curseur
    void setCursorVisible(bool visible);
    

private:
    // État du backend
    bool m_initialized{false};
    
    // Type pour stocker les curseurs chargés
    struct CursorData {
        ::Cursor cursor;
        std::string path;
        int size;
        
        CursorData() : cursor(0), size(0) {}
        CursorData(::Cursor c, const std::string& p, int s) 
            : cursor(c), path(p), size(s) {}
    };

    // Structure pour la gestion des raccourcis X11
    struct ShortcutInfo {
        std::function<void()> callback;
        KeyCode keycode{0};
        unsigned int modifiers{0};
        guint keyval{0};
        GdkModifierType mods;
        
        ShortcutInfo() = default;
        ShortcutInfo(const std::function<void()>& cb, KeyCode kc, unsigned int m, 
                    guint kv, GdkModifierType md) 
            : callback(cb), keycode(kc), modifiers(m), keyval(kv), mods(md) {}
    };

    // Contexte X11
    Display* m_display{nullptr};
    Window m_rootWindow{0};
    Window m_xwindow{0};  // Fenêtre X11
    
    // Autres états
    bool m_useSystemCursors{true};
    bool m_cursorVisible{true};
    std::pair<int, int> m_cursorPosition{0, 0};
    
    // Stockage des raccourcis
    std::unordered_map<std::string, ShortcutInfo> m_shortcuts;
    
    // Stockage des curseurs personnalisés
    std::unordered_map<CursorType, CursorData> m_customCursors;
    
    // Curseur système original
    ::Cursor m_originalCursor;
    
    X11Backend(const X11Backend&) = delete;
    X11Backend& operator=(const X11Backend&) = delete;
    // Méthodes utilitaires
    bool loadCursorFromFile(CursorType type, 
                         const std::string& path, 
                         int size = 32,
                         int hotspotX = 0,
                         int hotspotY = 0);
    
    bool loadDefaultCursor(CursorType type = CursorType::Default);
    
    // Convertit notre type de curseur en curseur X11
    unsigned int toX11CursorType(CursorType type) const;
    
    // Applique un curseur à la fenêtre racine
    bool applyCursor(::Cursor cursor);
    
    // Initialise la fenêtre X11
    bool initializeXWindow();

private:
    // État du backend
    bool m_initialized{false};
    
    // Contexte X11
    Display* m_display{nullptr};
    Window m_rootWindow{0};
    Window m_xwindow{0};  // Fenêtre X11
    
    // Autres états
    bool m_useSystemCursors{true};
    bool m_cursorVisible{true};
    ::Cursor m_originalCursor{0};
    
    // Stockage des raccourcis
    std::unordered_map<std::string, ShortcutInfo> m_shortcuts;
    
    // Stockage des curseurs personnalisés
    std::unordered_map<CursorType, CursorData> m_customCursors;
};

// Restaurer les définitions de macros
#pragma pop_macro("None")
#pragma pop_macro("Bool")
#pragma pop_macro("Status")
#pragma pop_macro("True")
#pragma pop_macro("False")

} // namespace input
