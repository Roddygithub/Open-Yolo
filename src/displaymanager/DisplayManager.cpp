#include "displaymanager/DisplayManager.hpp"

// Standard C++
#include <cerrno>   // Pour errno
#include <cstring>  // Pour std::strerror
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdlib>  // Pour getenv

// X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

namespace input {

struct DisplayManager::Impl {
    std::vector<DisplayInfo> displays;
    const DisplayInfo* primaryDisplay = nullptr;
    bool followMouse = false;
    Display* xdisplay = nullptr;
    int windowWidth = 1024;
    int windowHeight = 768;
    bool isFullscreen = false;
    
    ~Impl() {
        if (xdisplay) {
            XCloseDisplay(xdisplay);
            xdisplay = nullptr;
        }
    }
    
    void updateDisplays() {
        displays.clear();
        primaryDisplay = nullptr;
        
        if (!xdisplay) {
            xdisplay = XOpenDisplay(nullptr);
            if (!xdisplay) {
                const char* display = getenv("DISPLAY");
                std::string errorMsg = "Impossible d'ouvrir la connexion au serveur X";
                if (display) {
                    errorMsg += " (DISPLAY=" + std::string(display) + ")";
                }
                errorMsg += ": " + std::string(std::strerror(errno));
                throw std::runtime_error(errorMsg);
            }
        }
        
        Window root = DefaultRootWindow(xdisplay);
        if (!root) {
            throw std::runtime_error("Impossible d'obtenir la fenêtre racine");
        }
        
        int event_base, error_base;
        if (!XRRQueryExtension(xdisplay, &event_base, &error_base)) {
            throw std::runtime_error("L'extension RandR n'est pas disponible");
        }
        
        XRRScreenResources* res = XRRGetScreenResources(xdisplay, root);
        if (!res) {
            throw std::runtime_error("Impossible d'obtenir les ressources d'écran");
        }
        
        // Obtenir le moniteur principal
        RROutput primary = None;
        int major, minor;
        if (XRRQueryVersion(xdisplay, &major, &minor)) {
            if (major > 1 || (major == 1 && minor >= 3)) {
                primary = XRRGetOutputPrimary(xdisplay, root);
            }
        }
        
        // Parcourir tous les écrans
        for (int i = 0; i < res->noutput; i++) {
            XRROutputInfo* output_info = XRRGetOutputInfo(xdisplay, res, res->outputs[i]);
            if (!output_info) {
                std::cerr << "Erreur lors de la récupération des informations de sortie pour l'écran " << i << std::endl;
                continue;
            }
            
            if (output_info->connection == RR_Disconnected) {
                XRRFreeOutputInfo(output_info);
                continue;
            }
            
            if (!output_info->crtc) {
                XRRFreeOutputInfo(output_info);
                continue;
            }
            
            XRRCrtcInfo* crtc_info = XRRGetCrtcInfo(xdisplay, res, output_info->crtc);
            if (!crtc_info) {
                XRRFreeOutputInfo(output_info);
                continue;
            }
            
            DisplayInfo info;
            info.id = i;
            info.x = crtc_info->x;
            info.y = crtc_info->y;
            info.width = crtc_info->width;
            info.height = crtc_info->height;
            info.scale = 1.0f; // Par défaut, à moins qu'on ne trouve une meilleure façon de l'obtenir
            info.isPrimary = (res->outputs[i] == primary);
            
            // Récupérer le nom du moniteur
            info.name = output_info->name ? output_info->name : ("Écran " + std::to_string(i + 1));
            
            // Stocker le pointeur vers les infos X11
            info.ptr = output_info;
            
            // Ajouter à la liste des écrans
            displays.push_back(info);
            
            // Mettre à jour l'écran principal si nécessaire
            if (info.isPrimary) {
                primaryDisplay = &displays.back();
            }
            
            XRRFreeCrtcInfo(crtc_info);
            // Ne pas libérer output_info ici, on le garde dans le ptr
        }
        
        XRRFreeScreenResources(res);
    }
    
    // Implémentation des autres méthodes de l'interface
    const std::vector<DisplayInfo>& getDisplays() const {
        return displays;
    }
    
    const DisplayInfo* getPrimaryDisplay() const {
        return primaryDisplay;
    }
    
    const DisplayInfo* getDisplayForWindow(int windowX, int windowY) const {
        for (const auto& display : displays) {
            if (windowX >= display.x && windowX < display.x + static_cast<int>(display.width) &&
                windowY >= display.y && windowY < display.y + static_cast<int>(display.height)) {
                return &display;
            }
        }
        return nullptr;
    }
    
    void setFollowMouse(bool enable) {
        followMouse = enable;
        // Logique supplémentaire pour activer/désactiver le suivi de la souris
        if (enable) {
            std::cout << "Suivi de la souris activé" << std::endl;
        } else {
            std::cout << "Suivi de la souris désactivé" << std::endl;
        }
    }
    
    bool isFollowingMouse() const {
        return followMouse;
    }
    
    void setWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
    }
    
    void setFullscreen(bool fullscreen) {
        isFullscreen = fullscreen;
    }
    
    void beginFrame() {
        // Début de la frame de rendu
        // Peut être utilisé pour des opérations de pré-rendu
    }
    
    void endFrame() {
        // Fin de la frame de rendu
        // Peut être utilisé pour le swap des buffers ou autres opérations de post-rendu
    }
};

// Implémentation des méthodes de la classe DisplayManager
DisplayManager::DisplayManager() : pImpl(std::make_unique<Impl>()) {}

DisplayManager::~DisplayManager() = default;

bool DisplayManager::initialize() {
    try {
        pImpl->updateDisplays();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation du gestionnaire d'écrans: " << e.what() << std::endl;
        return false;
    }
}

void DisplayManager::updateDisplays() {
    pImpl->updateDisplays();
}

const std::vector<DisplayInfo>& DisplayManager::getDisplays() const {
    if (!pImpl) {
        throw std::runtime_error("DisplayManager not initialized");
    }
    return pImpl->displays;
}

const DisplayInfo* DisplayManager::getPrimaryDisplay() const {
    if (!pImpl) {
        return nullptr;
    }
    return pImpl->primaryDisplay;
}

const DisplayInfo* DisplayManager::getDisplayForWindow(int windowX, int windowY) const {
    return pImpl->getDisplayForWindow(windowX, windowY);
}

void DisplayManager::setFollowMouse(bool enable) {
    pImpl->setFollowMouse(enable);
}

bool DisplayManager::isFollowingMouse() const {
    return pImpl->isFollowingMouse();
}

void DisplayManager::setWindowSize(int width, int height) {
    pImpl->setWindowSize(width, height);
}

void DisplayManager::setFullscreen(bool fullscreen) {
    pImpl->setFullscreen(fullscreen);
}

void DisplayManager::beginFrame() {
    pImpl->beginFrame();
}

void DisplayManager::endFrame() {
    pImpl->endFrame();
}

} // namespace input
