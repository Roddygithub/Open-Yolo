#pragma once

// Standard C++
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

// GTKmm
#include <gdk/gdkx.h>
#include <gtkmm.h>

// Project headers
#include "../../cursormanager/CursorManager.hpp"
#include "../CursorError.hpp"
#include "../InputBackend.hpp"

// Utiliser le type CursorType depuis l'espace de noms cursor_manager
using CursorType = cursor_manager::CursorManager::CursorType;

// Définition de la macro LOG_ERROR si non définie
#ifndef LOG_ERROR
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl;
#endif

// Déclaration anticipée de la classe Display X11
typedef struct _XDisplay Display;

// Déclaration anticipée du type Window X11
typedef unsigned long Window;

// Déclaration anticipée du type Cursor X11
typedef unsigned long Cursor;

// Déclaration anticipée du type KeyCode X11
typedef unsigned char KeyCode;

// Sauvegarder les définitions de macros problématiques
#pragma push_macro("None")
#pragma push_macro("Bool")
#pragma push_macro("Status")
#pragma push_macro("True")
#pragma push_macro("False")

// Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace input {

class X11Backend : public InputBackend {
public:
    explicit X11Backend(Display* display);
    ~X11Backend() override;

    // Implémentation de l'interface InputBackend
    bool isInitialized() const override { return m_initialized; }
    bool initialize() override;
    bool isAvailable() const override;
    void registerShortcut(const std::string& name, const std::string& accelerator,
                          KeyCallback callback) override;
    void unregisterShortcut(const std::string& name) override;
    bool handleKeyEvent(guint keyval, GdkModifierType mods) override;
    bool onKeyPressed(GdkEventKey* event) override;
    bool shutdown() override { return true; }  // Implémentation par défaut

    // Méthodes spécifiques à X11
    bool setCursor(CursorType type, const std::string& cursorPath = "", int size = 32,
                   int hotspotY = 0);
    void useSystemCursors(bool useSystem = true);
    void setCursorPosition(int x, int y);
    std::pair<int, int> getCursorPosition() const;
    void setCursorVisible(bool visible);

private:
    // Structure pour stocker les informations de curseur
    struct CursorData {
        ::Cursor cursor{0};
        std::string path;
        int size{0};

        CursorData() = default;
        CursorData(::Cursor c, const std::string& p, int s) : cursor(c), path(p), size(s) {}
    };

    // Structure pour la gestion des raccourcis X11
    struct ShortcutInfo {
        std::function<void()> callback;
        KeyCode keycode{0};
        unsigned int modifiers{0};
        guint keyval{0};
        GdkModifierType mods;

        ShortcutInfo() = default;
        ShortcutInfo(const std::function<void()>& cb, KeyCode kc, unsigned int m, guint kv,
                     GdkModifierType md)
            : callback(cb), keycode(kc), modifiers(m), keyval(kv), mods(md) {}
    };

    // Méthodes utilitaires
    bool loadCursorFromFile(CursorType type, const std::string& path, int size = 32,
                            int hotspotX = 0, int hotspotY = 0);
    bool loadDefaultCursor(CursorType type = CursorType::Default);
    unsigned int toX11CursorType(CursorType type) const;
    bool applyCursor(::Cursor cursor);
    bool initializeXWindow();

    // Membres de la classe
    bool m_initialized{false};
    Display* m_display{nullptr};
    Window m_rootWindow{0};
    Window m_xwindow{0};
    bool m_useSystemCursors{true};
    bool m_cursorVisible{true};
    ::Cursor m_originalCursor{0};
    std::unordered_map<std::string, ShortcutInfo> m_shortcuts;
    std::unordered_map<CursorType, CursorData> m_customCursors;

    // Suppression des constructeurs de copie et d'affectation
    X11Backend(const X11Backend&) = delete;
    X11Backend& operator=(const X11Backend&) = delete;
};

}  // namespace input

// Restaurer les définitions de macros
#pragma GCC diagnostic pop
#pragma pop_macro("None")
#pragma pop_macro("Bool")
#pragma pop_macro("Status")
#pragma pop_macro("True")
#pragma pop_macro("False")
