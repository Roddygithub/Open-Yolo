#include "displaymanager/DisplayManager.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <iostream>

struct DisplayManager::Impl {
    std::vector<DisplayInfo> displays;
    const DisplayInfo* primaryDisplay = nullptr;
    bool followMouse = false;
    
    void updateDisplays() {
        displays.clear();
        primaryDisplay = nullptr;
        
        auto display = Gdk::Display::get_default();
        if (!display) {
            std::cerr << "Erreur: Impossible d'obtenir le display GDK" << std::endl;
            return;
        }
        
        // Obtenir le nombre de moniteurs
        int n_monitors = display->get_n_monitors();
        
        for (int i = 0; i < n_monitors; i++) {
            auto monitor = display->get_monitor(i);
            if (!monitor) continue;
            
            DisplayInfo info;
            info.id = i;
            
            // Obtenir la géométrie du moniteur
            Gdk::Rectangle rect;
            monitor->get_geometry(rect);
            info.x = rect.get_x();
            info.y = rect.get_y();
            info.width = rect.get_width();
            info.height = rect.get_height();
            info.scale = monitor->get_scale_factor();
            info.isPrimary = monitor->is_primary();
            
            // Récupérer le nom du moniteur
            auto name = monitor->get_model();
            if (name.empty()) {
                name = "Écran " + std::to_string(i + 1);
            }
            info.name = name;
            
            // Stocker un pointeur vers le moniteur
            info.ptr = monitor->gobj();
            
            displays.push_back(info);
            
            if (info.isPrimary) {
                primaryDisplay = &displays.back();
            }
        }
    } // Ajout de cette accolade pour fermer la fonction updateDisplays()
};

DisplayManager::DisplayManager() : pImpl(std::make_unique<Impl>()) {}

DisplayManager::~DisplayManager() = default;

bool DisplayManager::initialize() {
    try {
        pImpl->updateDisplays();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation du DisplayManager: " << e.what() << std::endl;
        return false;
    }
}

void DisplayManager::updateDisplays() {
    pImpl->updateDisplays();
}

const std::vector<DisplayInfo>& DisplayManager::getDisplays() const {
    return pImpl->displays;
}

const DisplayInfo* DisplayManager::getPrimaryDisplay() const {
    return pImpl->primaryDisplay;
}

const DisplayInfo* DisplayManager::getDisplayForWindow(int windowX, int windowY) const {
    for (const auto& display : pImpl->displays) {
        if (windowX >= display.x && windowX < display.x + display.width &&
            windowY >= display.y && windowY < display.y + display.height) {
            return &display;
        }
    }
    return nullptr;
}

void DisplayManager::setFollowMouse(bool enable) {
    pImpl->followMouse = enable;
    // Logique supplémentaire pour activer/désactiver le suivi de la souris
    if (enable) {
        std::cout << "Suivi de la souris activé" << std::endl;
    } else {
        std::cout << "Suivi de la souris désactivé" << std::endl;
    }
}

bool DisplayManager::isFollowingMouse() const {
    return pImpl->followMouse;
}
