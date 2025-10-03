#include "../../include/input/InputManager.hpp"
#include "../../include/gui/MainWindow.hpp"
#include "backends/X11Backend.hpp" // Nous allons créer ce fichier ensuite
#include "backends/WaylandBackend.hpp"

#include <X11/Xlib.h>
// Standard C++
#include <algorithm> // Pour std::find_if
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

// GTKmm
#include <gdk/gdk.h> // Pour les constantes GDK
#include <gdk/gdkx.h> // Pour les fonctions X11
#include <gdkmm/device.h>
#include <gdkmm/display.h>
#include <gdkmm/displaymanager.h>
#include <gdkmm/seat.h>
#include <glibmm/ustring.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
 
InputManager::InputManager() : mainWindow_(nullptr) {
    // Détecter si nous sommes sur Wayland ou X11
    // GDK_IS_X11_DISPLAY est une macro pratique pour cela.
    auto display = Gdk::Display::get_default();
    if (display && GDK_IS_X11_DISPLAY(display->gobj())) {
        std::cout << "Backend X11 détecté. Initialisation de X11Backend." << std::endl;
        backend_ = std::make_unique<input::X11Backend>();
    } else if (display && GDK_IS_WAYLAND_DISPLAY(display->gobj())) {
        std::cout << "Backend Wayland détecté. Initialisation de WaylandBackend." << std::endl;
        backend_ = std::make_unique<input::WaylandBackend>();
        // std::cerr << "Le backend Wayland n'est pas encore implémenté." << std::endl;
    } else {
        std::cerr << "Serveur d'affichage non supporté." << std::endl;
    }
}

InputManager::~InputManager() {
    // Nettoyage des gestionnaires d'événements
    shortcuts_.clear();
}

bool InputManager::initialize() {
    // L'initialisation est déléguée au backend.
    // On lui passe un callback pour qu'il puisse nous notifier des raccourcis pressés.
    return backend_ ? backend_->initialize() : false;
}

void InputManager::setupGTKIntegration(MainWindow* window, std::shared_ptr<cursor_manager::CursorManager> cursorManager) {
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

bool InputManager::removeShortcut(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Le nom du raccourci ne peut pas être vide");
    }

    // Déléguer la suppression au backend
    return backend_ ? backend_->unregisterShortcut(name) : false;
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
