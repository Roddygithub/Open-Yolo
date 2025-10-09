// Standard C++
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>

// GTKmm (doit être avant X11)
#include <gdk/gdkx.h>
#include <gtkmm.h>

// X11 (doit être après GTKmm)
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysym.h>

// Project headers
#include "cursormanager/CursorManager.hpp"
#include "input/CursorError.hpp"
#include "input/backends/X11Backend.hpp"

using std::begin;
using std::end;
using CursorType = cursor_manager::CursorManager::CursorType;

namespace fs = std::filesystem;

// Définition de XC_crossed_circle si non définie
#ifndef XC_crossed_circle
#define XC_crossed_circle 116
#endif

namespace input {

X11Backend::X11Backend(Display* display)
    : m_initialized(false),
      m_display(display),
      m_rootWindow(0),
      m_xwindow(0),
      m_useSystemCursors(true),
      m_cursorVisible(true),
      m_originalCursor(0) {}

X11Backend::~X11Backend() {
    if (m_initialized) {
        // Libérer tous les curseurs personnalisés
        for (auto it = m_customCursors.begin(); it != m_customCursors.end(); ++it) {
            if (it->second.cursor) {
                XFreeCursor(m_display, it->second.cursor);
            }
        }

        if (m_display) {
            XCloseDisplay(m_display);
        }
    }
}

bool X11Backend::initialize() {
    if (m_initialized) {
        return true;
    }

    // Ouvrir la connexion au serveur X
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        std::cerr << "Erreur: Impossible d'ouvrir la connexion au serveur X11" << std::endl;
        return false;
    }

    // Vérifier l'extension XInput2
    int xi_opcode, event, error;
    if (!XQueryExtension(m_display, "XInputExtension", &xi_opcode, &event, &error)) {
        std::cerr << "Erreur: L'extension XInput2 n'est pas disponible" << std::endl;
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    // Enregistrer les événements d'entrée
    Window root = DefaultRootWindow(m_display);
    if (!root) {
        std::cerr << "Erreur: Impossible d'obtenir la fenêtre racine" << std::endl;
        m_display = nullptr;
        return false;
    }

    // Récupérer la fenêtre racine
    m_rootWindow = DefaultRootWindow(m_display);
    if (!m_rootWindow) {
        if (m_xwindow) {
            XDestroyWindow(m_display, m_xwindow);
            m_xwindow = 0;
        }
        return false;
    }

    // Vérifier l'extension XFixes
    int event_base, error_base;
    if (!XFixesQueryExtension(m_display, &event_base, &error_base)) {
        std::cerr << "Erreur: L'extension XFixes n'est pas disponible" << std::endl;
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    // Initialiser la fenêtre X11
    if (!initializeXWindow()) {
        std::cerr << "Erreur: Impossible d'initialiser la fenêtre X11" << std::endl;
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    m_initialized = true;
    return true;
}

void X11Backend::setCursorPosition(int x, int y) {
    if (!m_initialized || !m_xwindow) {
        return;
    }

    XWarpPointer(m_display, None, m_xwindow, 0, 0, 0, 0, x, y);
    XFlush(m_display);
}

std::pair<int, int> X11Backend::getCursorPosition() const {
    if (!m_display || !m_xwindow) {
        return {0, 0};
    }

    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    XQueryPointer(m_display, m_xwindow, &root, &child, &rootX, &rootY, &winX, &winY, &mask);

    return {rootX, rootY};
}

void X11Backend::setCursorVisible(bool visible) {
    if (!m_initialized || !m_xwindow) {
        return;
    }

    if (visible) {
        XUndefineCursor(m_display, m_xwindow);
    } else {
        XDefineCursor(m_display, m_xwindow, m_originalCursor);
    }
    XFlush(m_display);
}

bool X11Backend::applyCursor(::Cursor cursor) {
    if (!m_display || !m_xwindow) {
        return false;
    }

    // Définir le curseur pour la fenêtre
    XDefineCursor(m_display, m_xwindow, cursor);
    XFlush(m_display);
    
    return true;
}

bool X11Backend::handleKeyEvent(guint keyval, GdkModifierType mods) {
    if (!m_initialized) {
        return false;
    }

    // Parcourir tous les raccourcis enregistrés
    for (const auto& [name, shortcut] : m_shortcuts) {
        // Vérifier si la touche et les modificateurs correspondent
        if (shortcut.keyval == keyval &&
            shortcut.mods ==
                (mods & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK))) {
            // Appeler le callback du raccourci
            if (shortcut.callback) {
                shortcut.callback();
                return true;
            }
        }
    }

    return false;
}

bool X11Backend::onKeyPressed(GdkEventKey* event) {
    if (!m_initialized || !event) {
        return false;
    }
    return handleKeyEvent(event->keyval, static_cast<GdkModifierType>(event->state));
}

bool X11Backend::isAvailable() const {
    // Vérifier si X11 est disponible
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        XCloseDisplay(display);
        return true;
    }
    return false;
}

void X11Backend::unregisterShortcut(const std::string& name) {
    if (!m_initialized)
        return;
    m_shortcuts.erase(name);
}

bool X11Backend::initializeXWindow() {
    if (!m_display)
        return false;

    // Créer une fenêtre X11 simple
    m_rootWindow = DefaultRootWindow(m_display);
    if (!m_rootWindow) {
        std::cerr << "Erreur: Impossible d'obtenir la fenêtre racine" << std::endl;
        return false;
    }

    // Créer une fenêtre enfant invisible pour gérer les événements
    XSetWindowAttributes attr;
    attr.override_redirect = True;
    attr.event_mask =
        KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

    m_xwindow = XCreateWindow(m_display, m_rootWindow, 0, 0, 1, 1,  // Position et taille minimales
                              0,                                    // Largeur de la bordure
                              CopyFromParent,                       // Profondeur
                              InputOutput,                          // Classe
                              CopyFromParent,                       // Visual
                              CWOverrideRedirect | CWEventMask, &attr);

    if (!m_xwindow) {
        std::cerr << "Erreur: Impossible de créer la fenêtre X11" << std::endl;
        return false;
    }

    // Créer un curseur flèche par défaut
    m_originalCursor = XCreateFontCursor(m_display, XC_left_ptr);
    if (!m_originalCursor) {
        std::cerr << "Erreur: Impossible de créer le curseur par défaut" << std::endl;
        XDestroyWindow(m_display, m_xwindow);
        m_xwindow = 0;
        return false;
    }

    return true;
}

}  // namespace input
