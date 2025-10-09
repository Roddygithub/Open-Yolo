#pragma once

// Standard C++
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

// Project headers
#include "input/CursorError.hpp"

// Forward declarations
namespace openyolo {
namespace input {
    class CursorError;
}
}

// Cairo
#include <cairomm/context.h>

namespace cursor_manager {

class CursorManager {
public:
    // Types de curseurs supportés
    enum class CursorType {
        Default,    // Curseur système par défaut
        Pointer,    // Pointeur standard (flèche)
        Text,       // Curseur de texte (I-beam)
        Hand,       // Main pour les liens
        ResizeAll,  // Flèches dans les 4 directions
        Help,       // Point d'interrogation
        Busy,       // Sablier/roue de chargement
        NotAllowed, // Cercle barré
        Move,       // Flèches dans les 4 directions avec main
        Count       // Nombre total de types de curseurs
    };

public:
    CursorManager();
    virtual ~CursorManager();
    
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, double scale);
    
    // Gestion du rendu
    void update();
    
    // Getters/Setters
    void setScale(float scale);
    float getScale() const;
    
    void setAnimationSpeed(int fps);
    int getAnimationSpeed() const;
    
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    bool isVisible() const;
    bool isInitialized() const;
    
    const std::string& getCurrentCursorPath() const;
    void setCursorPath(const std::string& path);

    // Dummy methods to fix compilation in main.cpp
    bool initialize() { isInitialized_ = true; return true; }
    void render() {}
    void setSize(float size) { setScale(size); }
    bool enableShortcuts(bool enable) { return true; }
    void updateShortcuts() {}

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Variables membres privées
    float scale_ = 1.0f;
    int targetFps_ = 60;
    std::string currentCursorPath_;
    bool isVisible_ = true;
    bool isInitialized_ = false;
};

} // namespace cursor_manager
