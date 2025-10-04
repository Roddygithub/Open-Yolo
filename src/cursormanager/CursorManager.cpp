#include "../../include/cursormanager/CursorManager.hpp"
#include "../../include/input/CursorError.hpp"

// Standard C++
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Cairo
#include <cairomm/context.h>

namespace cursor_manager {

class CursorManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    void draw(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, double scale) {
        if (!enabled_ || !visible_) {
            return;
        }

        try {
            // Sauvegarder l'état actuel du contexte
            cr->save();
            
            // Définir la couleur et dessiner un cercle comme curseur de test
            cr->set_source_rgba(1.0, 0.0, 0.0, 0.8); // Rouge semi-transparent
            cr->arc(x, y, 10.0 * scale, 0.0, 2.0 * M_PI);
            cr->fill();
            
            // Restaurer l'état du contexte
            cr->restore();
        } catch (const std::exception& e) {
            std::cerr << "Erreur lors du rendu du curseur: " << e.what() << std::endl;
        }
    }

    bool enabled_ = true;
    bool visible_ = true;
    float scale_ = 1.0f;
    int targetFps_ = 60;
    std::string currentCursorPath_;
};

CursorManager::CursorManager() : pImpl(std::make_unique<Impl>()) {}
CursorManager::~CursorManager() = default;

void CursorManager::draw(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, double scale) {
    if (pImpl) {
        pImpl->draw(cr, x, y, scale);
    }
}

void CursorManager::setScale(float scale) { 
    if (pImpl) {
        pImpl->scale_ = scale > 0 ? scale : 1.0f; 
    }
}

float CursorManager::getScale() const { 
    return pImpl ? pImpl->scale_ : 1.0f; 
}

void CursorManager::setAnimationSpeed(int fps) { 
    if (pImpl) {
        pImpl->targetFps_ = fps > 0 ? fps : 60; 
    }
}

void CursorManager::setEnabled(bool enabled) {
    if (pImpl) {
        pImpl->enabled_ = enabled;
    }
}

bool CursorManager::isEnabled() const {
    return pImpl ? pImpl->enabled_ : false;
}

bool CursorManager::isVisible() const {
    return pImpl ? pImpl->visible_ : false;
}

bool CursorManager::isInitialized() const {
    return pImpl != nullptr;
}

const std::string& CursorManager::getCurrentCursorPath() const {
    static const std::string empty;
    return pImpl ? pImpl->currentCursorPath_ : empty;
}

void CursorManager::setCursorPath(const std::string& path) {
    if (pImpl) {
        pImpl->currentCursorPath_ = path;
    }
}
int CursorManager::getAnimationSpeed() const { 
    return pImpl ? pImpl->targetFps_ : 60; 
}

} // namespace cursor_manager