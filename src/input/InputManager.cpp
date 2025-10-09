#include "../../include/input/InputManager.hpp"

#include <gdk/gdk.h>
#include <gdk/gdkwayland.h>
#include <gdk/gdkx.h>
#include <gtkmm.h>
#include <X11/Xlib.h>

#include <iostream>

#include "../../include/input/backends/WaylandBackend.hpp"
#include "../../include/input/backends/X11Backend.hpp"

// Définition des macros de journalisation si non définies
#ifndef LOG_DEBUG
#define LOG_DEBUG(msg)                                 \
    do {                                               \
        std::cout << "[DEBUG] " << (msg) << std::endl; \
    } while (0)
#endif

#ifndef LOG_INFO
#define LOG_INFO(msg)                                 \
    do {                                              \
        std::cout << "[INFO] " << (msg) << std::endl; \
    } while (0)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(msg)                                 \
    do {                                               \
        std::cerr << "[ERROR] " << (msg) << std::endl; \
    } while (0)
#endif

namespace input {

InputManager::InputManager(std::shared_ptr<displaymanager::DisplayManager> displayManager)
    : displayManager_(displayManager),
      mainWindow_(nullptr),
      cursorManager_(nullptr),
      backend_(nullptr),
      initialized_(false) {
    // Initialisation du backend en fonction de l'environnement
    const char* waylandDisplay = getenv("WAYLAND_DISPLAY");
    const char* x11Display = getenv("DISPLAY");

    if (waylandDisplay != nullptr) {
        LOG_INFO("Initialisation du backend Wayland");
        // Création du backend Wayland avec le gestionnaire d'entrée
        backend_ = std::make_unique<WaylandBackend>(this);
    } else if (x11Display != nullptr) {
        LOG_INFO("Initialisation du backend X11");
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            LOG_ERROR("Impossible d'ouvrir la connexion X11");
            return;
        }
        backend_ = std::make_unique<X11Backend>(display);
    } else {
        LOG_ERROR("Impossible de déterminer l'environnement d'affichage (X11 ou Wayland)");
        return;
    }

    if (backend_) {
        initialized_ = backend_->initialize();
        if (!initialized_) {
            LOG_ERROR("Échec de l'initialisation du backend d'entrée");
            backend_.reset();
        }
    }
}

bool InputManager::onKeyPressed(GdkEventKey* event) {
    if (!backend_) {
        return false;
    }

    // Convertir l'événement GdkEventKey en code de touche et modificateurs
    guint keyval = event->keyval;
    GdkModifierType mods = static_cast<GdkModifierType>(event->state);
    return backend_->handleKeyEvent(keyval, mods);
}

void InputManager::setupGTKIntegration(
    Gtk::Window* window, std::shared_ptr<cursor_manager::CursorManager> cursorManager) {
    if (!window) {
        LOG_ERROR("Erreur: la fenêtre GTK est nulle");
        return;
    }

    // Enregistrer la fenêtre principale
    mainWindow_ = window;

    // Enregistrer le gestionnaire de curseur
    cursorManager_ = cursorManager;

    // Configurer les signaux de la fenêtre principale
    window->signal_key_press_event().connect(sigc::mem_fun(*this, &InputManager::onKeyPressed),
                                             false);

    LOG_INFO("Intégration GTK configurée avec succès");
}

bool InputManager::parseAccelerator(const std::string& accel, guint& key,
                                    Gdk::ModifierType& mods) const {
    if (accel.empty()) {
        LOG_ERROR("Accelerator string is empty");
        return false;
    }

    // Initialiser les sorties
    key = 0;
    mods = Gdk::ModifierType(0);

    try {
        // Parser l'accélérateur avec GTK
        guint keyval = 0;
        GdkModifierType mods_type = GdkModifierType(0);

        gtk_accelerator_parse(accel.c_str(), &keyval, &mods_type);

        if (keyval == 0 || keyval == GDK_KEY_VoidSymbol) {
            LOG_ERROR("Invalid key value in accelerator: " + accel);
            return false;
        }

        // Convertir les modificateurs GDK vers Gdk::ModifierType
        Gdk::ModifierType result_mods = Gdk::ModifierType(0);

        if (mods_type & GDK_SHIFT_MASK) {
            result_mods = static_cast<Gdk::ModifierType>(result_mods | Gdk::SHIFT_MASK);
        }
        if (mods_type & GDK_CONTROL_MASK) {
            result_mods = static_cast<Gdk::ModifierType>(result_mods | Gdk::CONTROL_MASK);
        }
        if (mods_type & GDK_MOD1_MASK) {  // Alt key
            result_mods = static_cast<Gdk::ModifierType>(result_mods | Gdk::MOD1_MASK);
        }
        if (mods_type & GDK_SUPER_MASK) {
            result_mods = static_cast<Gdk::ModifierType>(result_mods | Gdk::SUPER_MASK);
        }

        key = keyval;
        mods = result_mods;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Exception in parseAccelerator: ") + e.what());
        return false;
    } catch (...) {
        LOG_ERROR("Unknown exception in parseAccelerator");
        return false;
    }
}

}  // namespace input
