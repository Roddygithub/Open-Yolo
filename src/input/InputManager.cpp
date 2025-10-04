// Standard C++
#include <cstring>     // Pour strcmp, etc.
#include <functional>  // Pour std::function
#include <iostream>    // Pour std::cerr, std::endl
#include <memory>      // Pour std::shared_ptr
#include <stdexcept>   // Pour std::runtime_error
#include <string>      // Pour std::string

// GTKmm (doit être avant X11)
#include <gtkmm.h>                // Pour les widgets GTK
#include <gdk/gdk.h>              // Pour les fonctions GDK de bas niveau
#include <gdkmm/device.h>         // Pour Gdk::Device
#include <gdkmm/display.h>        // Pour Gdk::Display
#include <gdkmm/displaymanager.h> // Pour Gdk::DisplayManager
#include <gdkmm/seat.h>           // Pour Gdk::Seat

// X11 (doit être après GTKmm)
#include <gdk/gdkx.h>             // Pour les fonctions spécifiques à X11
#include <X11/Xlib.h>             // Pour les fonctions X11

// Project headers
#include "../../include/input/InputManager.hpp"
#include "../../include/input/backends/X11Backend.hpp"
#include "../../include/gui/MainWindow.hpp"
#ifdef HAVE_WAYLAND
#include "../../include/input/backends/WaylandBackend.hpp"
#endif

namespace input {

InputManager::InputManager() : mainWindow_(nullptr), backend_(nullptr) {
    try {
        // Détecter si nous sommes sur X11
        auto display = Gdk::Display::get_default();
        if (!display) {
            throw std::runtime_error("Impossible d'obtenir le display GDK par défaut");
        }

        std::cerr << "Initialisation du gestionnaire d'entrée..." << std::endl;
        
        // Pour l'instant, nous utilisons uniquement le backend X11
        std::cerr << "Création du backend X11..." << std::endl;
        backend_ = new X11Backend();
        if (!backend_) {
            throw std::runtime_error("Échec de la création du backend X11");
        }

        // Initialiser le backend
        if (!backend_->initialize()) {
            delete backend_;
            backend_ = nullptr;
            throw std::runtime_error("Échec de l'initialisation du backend X11");
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur dans InputManager: " << e.what() << std::endl;
        if (backend_) {
            delete backend_;
            backend_ = nullptr;
        }
        throw;
    }
}

InputManager::~InputManager() {
    // Nettoyer le backend
    delete backend_;
    backend_ = nullptr;
    
    // Nettoyage des gestionnaires d'événements
    shortcuts_.clear();
}

bool InputManager::initialize() {
    if (!backend_) {
        std::cerr << "Erreur: Aucun backend d'entrée n'est disponible." << std::endl;
        return false;
    }
    
    try {
        // Initialisation du backend
        if (!backend_->initialize()) {
            std::cerr << "Échec de l'initialisation du backend d'entrée." << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation du backend d'entrée: " << e.what() << std::endl;
        return false;
    }
}

void InputManager::setupGTKIntegration(Gtk::Window* window, std::shared_ptr<cursor_manager::CursorManager> cursorManager) {
    if (!window) {
        throw std::invalid_argument("La fenêtre principale ne peut pas être nulle");
    }

    if (backend_) {
        // Fournir la fenêtre au backend (surtout utile pour Wayland)
        backend_->setWindow(window);
    }

    mainWindow_ = window;
    cursorManager_ = cursorManager;
    
    // Dans GTKMM 3.0, on connecte directement les signaux de la fenêtre
    window->signal_key_press_event().connect(
        sigc::mem_fun(*this, &InputManager::onKeyPressed), false);
}

bool InputManager::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    // Déléguer l'enregistrement au backend
    if (backend_) {
        return backend_->registerShortcut(name, accelerator, callback);
    }
    
    return false;
}

bool InputManager::onKeyPressed(GdkEventKey* event) {
    if (backend_) {
        return backend_->onKeyPressed(event);
    }
    
    // Laisser l'événement se propager
    return false;
}

void InputManager::removeShortcut(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Le nom du raccourci ne peut pas être vide");
    }

    // Déléguer la suppression au backend
    if (backend_) {
        backend_->unregisterShortcut(name);
    }
}

bool InputManager::parseAccelerator(const std::string& accel, guint& key, Gdk::ModifierType& mods) const {
    if (accel.empty()) return false;
    
    // Initialiser les modificateurs
    mods = Gdk::ModifierType(0);
    
    // Parser l'accélérateur avec GTK
    guint keyval = 0;
    GdkModifierType mods_type = GdkModifierType(0);
    
    gtk_accelerator_parse(accel.c_str(), &keyval, &mods_type);
    
    if (keyval == 0) {
        return false;
    }
    
    // Convertir les modificateurs
    Gdk::ModifierType result_mods = Gdk::ModifierType(0);
    
    // Mapping des modificateurs GDK vers Gdk::ModifierType
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
}

} // namespace input
