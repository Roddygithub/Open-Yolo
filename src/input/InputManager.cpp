#include "../../include/input/InputManager.hpp"
#include "../../include/gui/MainWindow.hpp"
#include <iostream>
#include <sstream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gdkmm/display.h>
#include <gdkmm/device.h>
#include <gdkmm/displaymanager.h>
#include <gdkmm/seat.h>
#include <gdk/gdk.h> // Pour les constantes GDK
#include <gdk/gdkx.h> // Pour les fonctions X11
#include <algorithm> // Pour std::find_if

InputManager::InputManager() 
    : mainWindow_(nullptr) {
    
    // Initialisation du gestionnaire d'affichage
    display_ = Gdk::Display::get_default();
    if (display_) {
        // Obtenir le gestionnaire de périphériques
        deviceManager_ = display_->get_device_manager();
        
        if (deviceManager_) {
            // Dans GTK3, nous devons surveiller manuellement les changements de périphériques
            // car les signaux signal_device_added/removed ne sont pas disponibles
            // Nous allons initialiser les périphériques maintenant
            initializeInputDevices();
            
            // Planifier une vérification périodique des périphériques
            Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &InputManager::checkForDeviceChanges),
                2000); // Vérifier toutes les 2 secondes
        }
    }
}

InputManager::~InputManager() {
    // Nettoyage des gestionnaires d'événements
    shortcuts_.clear();
}

bool InputManager::initialize() {
    // L'initialisation est déjà effectuée dans le constructeur
    // Cette méthode existe pour la compatibilité avec l'interface
    return display_ && deviceManager_;
}

void InputManager::setupGTKIntegration(MainWindow* window, std::shared_ptr<cursor_manager::CursorManager> cursorManager) {
    if (!window) {
        throw std::invalid_argument("La fenêtre principale ne peut pas être nulle");
    }

    mainWindow_ = window;
    cursorManager_ = cursorManager;
    
    // Dans GTKMM 3.0, on connecte directement les signaux de la fenêtre
    window->signal_key_press_event().connect(
        sigc::mem_fun(*this, &InputManager::onKeyPressed), false);
    
    // Initialiser les périphériques connectés
    initializeInputDevices();
}

bool InputManager::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    if (name.empty()) {
        throw std::invalid_argument("Le nom du raccourci ne peut pas être vide");
    }
    
    // Vérifier si le raccourci existe déjà
    {
        std::lock_guard<std::mutex> lock(shortcutsMutex_);
        if (shortcuts_.find(name) != shortcuts_.end()) {
            return false; // Le raccourci existe déjà
        }
    }
    
    // Parser l'accélérateur
    guint key = 0;
    Gdk::ModifierType mods;
    if (!parseAccelerator(accelerator, key, mods)) {
        std::cerr << "Format d'accélérateur invalide: " << accelerator << std::endl;
        return false;
    }
    
    // Ajouter le raccourci
    {
        std::lock_guard<std::mutex> lock(shortcutsMutex_);
        shortcuts_.emplace(name, Shortcut{accelerator, key, mods, callback});
    }
    
    return true;
}

bool InputManager::onKeyPressed(GdkEventKey* event) {
    if (!event) return false;
    
    // Convertir l'état des modificateurs pour la correspondance
    Gdk::ModifierType effective_mods = static_cast<Gdk::ModifierType>(event->state & (
        GDK_SHIFT_MASK | 
        GDK_CONTROL_MASK | 
        GDK_MOD1_MASK |  // Alt key in GTK3
        GDK_SUPER_MASK));

    // Vérifier si un raccourci correspond à la combinaison de touches
    for (const auto& pair : shortcuts_) {
        const auto& name = pair.first;
        const auto& shortcut = pair.second;
        
        if (shortcut.accelKey == event->keyval && shortcut.mods == effective_mods) {
            // Exécuter le callback du raccourci
            if (shortcut.callback) {
                shortcut.callback();
                return true; // Événement traité
            }
        }
    }
    
    // Laisser l'événement se propager
    return false;
}

void InputManager::removeShortcut(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Le nom du raccourci ne peut pas être vide");
    }

    std::lock_guard<std::mutex> lock(shortcutsMutex_);
    
    auto it = shortcuts_.find(name);
    if (it != shortcuts_.end()) {
        shortcuts_.erase(it);
        std::cout << "Raccourci supprimé: " << name << std::endl;
    } else {
        std::cerr << "Avertissement: Tentative de supprimer un raccourci inexistant: " 
                 << name << std::endl;
        throw std::out_of_range("Le raccourci spécifié n'existe pas");
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

void InputManager::initializeInputDevices() {
    if (!deviceManager_) return;
    
    try {
        // Obtenir tous les périphériques maîtres (claviers et souris)
        auto devices = deviceManager_->list_devices(Gdk::DEVICE_TYPE_MASTER);
        
        // Filtrer et ajouter uniquement les claviers
        for (const auto& device : devices) {
            if (device->get_source() == Gdk::SOURCE_KEYBOARD) {
                std::lock_guard<std::mutex> lock(devicesMutex_);
                keyboardDevices_.insert(device);
                std::cout << "Clavier initial détecté: " << device->get_name() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation des périphériques: " << e.what() << std::endl;
    }
}

bool InputManager::checkForDeviceChanges() {
    if (!deviceManager_) return true; // Continuer la vérification
    
    try {
        std::set<Glib::ustring> currentDevices;
        
        // Obtenir la liste actuelle des périphériques clavier
        auto devices = deviceManager_->list_devices(Gdk::DEVICE_TYPE_MASTER);
        
        // Identifier les périphériques clavier actuels
        for (const auto& device : devices) {
            if (device->get_source() == Gdk::SOURCE_KEYBOARD) {
                currentDevices.insert(device->get_name());
                
                // Vérifier si ce périphérique est nouveau
                std::lock_guard<std::mutex> lock(devicesMutex_);
                if (keyboardDevices_.find(device) == keyboardDevices_.end()) {
                    // Nouveau périphérique détecté
                    keyboardDevices_.insert(device);
                    std::cout << "Clavier connecté: " << device->get_name() << std::endl;
                }
            }
        }
        
        // Vérifier les périphériques manquants
        std::lock_guard<std::mutex> lock(devicesMutex_);
        for (auto it = keyboardDevices_.begin(); it != keyboardDevices_.end(); ) {
            if (currentDevices.find((*it)->get_name()) == currentDevices.end()) {
                std::cout << "Clavier déconnecté: " << (*it)->get_name() << std::endl;
                it = keyboardDevices_.erase(it);
            } else {
                ++it;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la vérification des périphériques: " << e.what() << std::endl;
    }
    
    return true; // Continuer la vérification périodique
}

void InputManager::onDeviceRemoved(const Glib::RefPtr<Gdk::Device>& device) {
    if (!device) return;
    
    try {
        // Vérifier si c'est un clavier
        if (device->get_source() == Gdk::SOURCE_KEYBOARD) {
            std::lock_guard<std::mutex> lock(devicesMutex_);
            
            // Vérifier si le périphérique est dans la liste avant de le supprimer
            auto it = keyboardDevices_.find(device);
            if (it != keyboardDevices_.end()) {
                std::cout << "Clavier déconnecté: " << device->get_name() << std::endl;
                keyboardDevices_.erase(it);
                
                // Vérifier s'il reste des claviers connectés
                if (keyboardDevices_.empty()) {
                    std::cerr << "Attention: Aucun clavier détecté!" << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la suppression du périphérique: " << e.what() << std::endl;
    }
}
