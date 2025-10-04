#pragma once

// Standard C++
#include <unordered_map>

// X11
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xcursor/Xcursor.h>

// Project headers
#include "../InputBackend.hpp"
#include "../CursorError.hpp"
#include "../../cursormanager/CursorManager.hpp"

// Alias pour le type de curseur
using CursorType = cursor_manager::CursorManager::CursorType;

namespace input {

class X11Backend : public InputBackend {
public:
    X11Backend();
    ~X11Backend();

    // Implémentation des méthodes de l'interface InputBackend
    bool initialize() override;
    void setWindow(Gtk::Window* window) override;
    bool registerShortcut(const std::string& name, const std::string& accelerator, 
                         const KeyCallback& callback) override;
    bool unregisterShortcut(const std::string& name) override;
    bool onKeyPressed(GdkEventKey* event) override;
    
    // Méthodes spécifiques à X11
    bool setCursor(CursorType type, 
                  const std::string& cursorPath = "", 
                  int size = 32,
                  int hotspotX = 0,
                  int hotspotY = 0);
    
    // Active/désactive l'utilisation des curseurs système
    void useSystemCursors(bool useSystem = true);
    
    // Met à jour la position du curseur
    void setCursorPosition(int x, int y);
    
    // Récupère la position actuelle du curseur
    std::pair<int, int> getCursorPosition() const;
    
    // Active/désactive la visibilité du curseur
    void setCursorVisible(bool visible);

    // Vérifie si l'initialisation a réussi
    bool isInitialized() const { return m_initialized; }

private:
    // Type pour stocker les curseurs chargés
    struct CursorData {
        ::Cursor cursor;
        std::string path;
        int size;
    };

    // Contexte X11
    Display* m_display;
    Window m_rootWindow;
    
    // État du backend
    bool m_initialized;
    bool m_useSystemCursors;
    
    // Stockage des curseurs personnalisés
    std::unordered_map<CursorType, CursorData> m_customCursors;
    
    // Curseur système original
    ::Cursor m_originalCursor;
    
    // Empêcher la copie
    X11Backend(const X11Backend&) = delete;
    X11Backend& operator=(const X11Backend&) = delete;
    
    // Méthodes utilitaires
    bool loadCursorFromFile(CursorType type, 
                           const std::string& path, 
                           int size = 32,
                           int hotspotX = 0,
                           int hotspotY = 0);
    
    bool loadDefaultCursor(CursorType type = CursorType::Default);
    
    // Convertit notre type de curseur en curseur X11
    unsigned int toX11CursorType(CursorType type) const;
    
    // Applique un curseur à la fenêtre racine
    bool applyCursor(::Cursor cursor);
};

} // namespace input
