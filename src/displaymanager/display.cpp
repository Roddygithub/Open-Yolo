#include "DisplayManager.hpp"
#include <SDL2/SDL.h>
#include <iostream>

struct DisplayManager::Impl {
    std::vector<DisplayInfo> displays;
    bool initialized = false;

    ~Impl() {
        if (initialized) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    }
};

DisplayManager::DisplayManager() : pImpl(std::make_unique<Impl>()) {}
DisplayManager::~DisplayManager() = default;

bool DisplayManager::initialize() {
    if (pImpl->initialized) {
        return true;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur d'initialisation SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    pImpl->initialized = true;
    updateDisplays();
    return true;
}

void DisplayManager::updateDisplays() {
    if (!pImpl->initialized) return;

    pImpl->displays.clear();
    int displayCount = SDL_GetNumVideoDisplays();
    
    for (int i = 0; i < displayCount; ++i) {
        SDL_Rect rect;
        if (SDL_GetDisplayBounds(i, &rect) != 0) {
            std::cerr << "Erreur lors de la récupération des informations de l'écran " << i << ": " 
                      << SDL_GetError() << std::endl;
            continue;
        }

        float dpi = 96.0f; // Valeur par défaut
        if (SDL_GetDisplayDPI(i, nullptr, &dpi, nullptr) != 0) {
            std::cerr << "Impossible de récupérer le DPI de l'écran " << i << ": " 
                      << SDL_GetError() << std::endl;
        }

        DisplayInfo info;
        info.id = i;
        info.x = rect.x;
        info.y = rect.y;
        info.width = rect.w;
        info.height = rect.h;
        info.scale = dpi / 96.0f; // Échelle basée sur le DPI
        info.isPrimary = (i == 0); // Le premier écran est considéré comme principal par défaut
        
        pImpl->displays.push_back(info);
    }
}

const std::vector<DisplayInfo>& DisplayManager::getDisplays() const {
    return pImpl->displays;
}

const DisplayInfo* DisplayManager::getPrimaryDisplay() const {
    if (pImpl->displays.empty()) {
        return nullptr;
    }
    return &pImpl->displays[0];
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
