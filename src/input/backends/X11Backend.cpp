// Standard C++
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iterator>  // Pour std::begin et std::end
#include <stdexcept>
#include <unordered_map>
#include <utility>   // Pour std::pair

// GTKmm (doit être avant X11)
#include <gtkmm.h>
#include <gdk/gdkx.h>

// X11 (doit être après GTKmm)
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xcursor/Xcursor.h>

// Project headers
#include "input/backends/X11Backend.hpp"
#include "input/CursorError.hpp"
#include "cursormanager/CursorManager.hpp"

// Pour std::begin et std::end avec les conteneurs
using std::begin;
using std::end;

// Alias pour le type de curseur
using CursorType = cursor_manager::CursorManager::CursorType;

namespace fs = std::filesystem;

// Définition de XC_crossed_circle si non définie
#ifndef XC_crossed_circle
#  define XC_crossed_circle 116
#endif

namespace input {

X11Backend::X11Backend()
    : m_display(nullptr)
    , m_rootWindow(0)
    , m_initialized(false)
    , m_useSystemCursors(true)
    , m_originalCursor(0) {
}

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
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    // Récupérer la fenêtre racine
    m_rootWindow = DefaultRootWindow(m_display);
    if (!m_rootWindow) {
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    // Vérifier l'extension XFixes
    int event_base, error_base;
    if (!XFixesQueryExtension(m_display, &event_base, &error_base)) {
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }

    XFixesSelectCursorInput(m_display, m_rootWindow, XFixesDisplayCursorNotifyMask);
    m_initialized = true;
    return true;
}

bool X11Backend::setCursor(CursorType type, const std::string& cursorPath, int size, int hotspotX, int hotspotY) {
    if (!m_initialized || m_useSystemCursors) {
        return false;
    }
    
    // Si le chemin est vide, on charge le curseur par défaut
    // Si le chemin est vide, on charge le curseur par défaut
    if (cursorPath.empty()) {
        return loadDefaultCursor(type);
    }
    
    return loadCursorFromFile(type, cursorPath, size, hotspotX, hotspotY);
}

void X11Backend::useSystemCursors(bool useSystem) {
    if (!m_initialized) return;
    
    m_useSystemCursors = useSystem;
    
    if (useSystem) {
        // Restaurer le curseur système par défaut (le curseur racine)
        XUndefineCursor(m_display, m_rootWindow);
    } else {
        // Appliquer le dernier curseur personnalisé pour chaque type
        for (auto it = m_customCursors.begin(); it != m_customCursors.end(); ++it) {
            if (it->second.cursor) {
                XDefineCursor(m_display, m_rootWindow, it->second.cursor);
            }
        }
    }
    
    XFlush(m_display);
}

void X11Backend::setCursorPosition(int x, int y) {
    if (!m_initialized) return;
    
    XWarpPointer(m_display, None, m_rootWindow, 0, 0, 0, 0, x, y);
    XFlush(m_display);
}

std::pair<int, int> X11Backend::getCursorPosition() const {
    if (!m_initialized) {
        return {0, 0};
    }

    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    
    XQueryPointer(m_display, m_rootWindow, &root, &child, 
                 &rootX, &rootY, &winX, &winY, &mask);
    
    return {rootX, rootY};
}

void X11Backend::setCursorVisible(bool visible) {
    if (!m_initialized) return;
    
    if (visible) {
        XUndefineCursor(m_display, m_rootWindow);
    } else {
        // Créer un curseur vide
        Pixmap bm_no;
        XColor black, dummy;
        Colormap colormap = DefaultColormap(m_display, DefaultScreen(m_display));
        XAllocNamedColor(m_display, colormap, "black", &black, &dummy);
        char bm[] = {0, 0, 0, 0, 0, 0, 0, 0};
        bm_no = XCreateBitmapFromData(m_display, m_rootWindow, bm, 8, 8);
        Cursor cursor = XCreatePixmapCursor(m_display, bm_no, bm_no, &black, &black, 0, 0);
        XDefineCursor(m_display, m_rootWindow, cursor);
        XFreeCursor(m_display, cursor);
        XFreePixmap(m_display, bm_no);
    }
    
    XFlush(m_display);
}

// Méthodes privées
bool X11Backend::loadCursorFromFile(CursorType type, 
                                   const std::string& path, 
                                   int size,
                                   int hotspotX,
                                   int hotspotY) {
    if (!m_initialized) return false;
    
    // Charger l'image du curseur
    XcursorImage* image = XcursorFilenameLoadImage(path.c_str(), size);
    if (!image) {
        return false;
    }
    
    // Mettre à jour le hotspot si spécifié
    if (hotspotX >= 0) image->xhot = hotspotX;
    if (hotspotY >= 0) image->yhot = hotspotY;
    
    // Créer le curseur
    Cursor cursor = XcursorImageLoadCursor(m_display, image);
    XcursorImageDestroy(image);
    
    if (!cursor) {
        return false;
    }
    
    // Stocker le curseur
    CursorData& data = m_customCursors[type];
    if (data.cursor) {
        XFreeCursor(m_display, data.cursor);
    }
    
    data.cursor = cursor;
    data.path = path;
    data.size = size;
    
    // Appliquer le curseur si on n'utilise pas les curseurs système
    if (!m_useSystemCursors) {
        XDefineCursor(m_display, m_rootWindow, cursor);
        XFlush(m_display);
    }
    
    return true;
}

bool X11Backend::loadDefaultCursor(CursorType type) {
    if (!m_initialized) return false;
    
    unsigned int cursorShape = toX11CursorType(type);
    Cursor cursor = XCreateFontCursor(m_display, cursorShape);
    
    if (!cursor) {
        return false;
    }
    
    // Stocker le curseur
    CursorData& data = m_customCursors[type];
    if (data.cursor) {
        XFreeCursor(m_display, data.cursor);
    }
    
    data.cursor = cursor;
    data.path = "default";
    data.size = 0;
    
    // Appliquer le curseur si on n'utilise pas les curseurs système
    if (!m_useSystemCursors) {
        XDefineCursor(m_display, m_rootWindow, cursor);
        XFlush(m_display);
    }
    
    return true;
}

unsigned int X11Backend::toX11CursorType(CursorType type) const {
    switch (type) {
        case CursorType::Pointer:    return XC_arrow;
        case CursorType::Text:       return XC_xterm;
        case CursorType::Hand:       return XC_hand2;
        case CursorType::ResizeAll:  return XC_fleur;
        case CursorType::Help:       return XC_question_arrow;
        case CursorType::Busy:       return XC_watch;
        case CursorType::NotAllowed: return XC_crossed_circle;
        case CursorType::Move:       return XC_fleur;
        case CursorType::Default:    
        default:                                    return XC_left_ptr;
    }
}

bool X11Backend::applyCursor(::Cursor cursor) {
    if (!m_initialized) return false;
    
    XDefineCursor(m_display, m_rootWindow, cursor);
    XFlush(m_display);
    return true;
}

// Implémentation des méthodes de l'interface InputBackend

// Implémentation des méthodes de l'interface InputBackend

void X11Backend::setWindow(Gtk::Window* window) {
    // Cette méthode est spécifique à X11, donc nous n'avons rien de spécial à faire ici
    // car nous avons déjà accès à la fenêtre racine via m_rootWindow
    (void)window; // Évite l'avertissement de paramètre inutilisé
}

bool X11Backend::registerShortcut(const std::string& name, const std::string& accelerator, 
                                 const KeyCallback& callback) {
    // Cette méthode est spécifique à X11 et nécessite une implémentation plus poussée
    // pour gérer les raccourcis clavier au niveau X11
    // Pour l'instant, nous utilisons la gestion des raccourcis de GTK
    // via les signaux de la fenêtre principale
    (void)name;
    (void)accelerator;
    (void)callback;
    return false;
}

bool X11Backend::unregisterShortcut(const std::string& name) {
    // Implémentation pour supprimer un raccourci
    (void)name;
    return false;
}

bool X11Backend::onKeyPressed(GdkEventKey* event) {
    // Cette méthode sera appelée lorsqu'une touche est pressée
    // Elle doit être connectée au signal key-press-event de GTK
    (void)event;
    return false;
}

} // namespace input
