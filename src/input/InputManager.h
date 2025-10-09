#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <gdkmm.h>
#include <gtkmm.h>

#include <functional>
#include <memory>
#include <string>

#include "InputBackend.hpp"

// Forward declarations
namespace cursor_manager {
class CursorManager;
}

namespace displaymanager {
class DisplayManager;
}

namespace input {

class InputManager {
public:
    InputManager() : InputManager(nullptr) {}
    explicit InputManager(std::shared_ptr<displaymanager::DisplayManager> displayManager);
    ~InputManager();

    // Initialise le gestionnaire d'entrée
    void initialize();

    // Enregistre un raccourci clavier global
    // name: nom unique du raccourci
    // accelerator: combinaison de touches au format GTK (ex: "<Control>s")
    // callback: fonction à appeler lorsque le raccourci est activé
    void registerShortcut(const std::string& name, const std::string& accelerator,
                          std::function<void()> callback);

    // Désenregistre un raccourci
    void unregisterShortcut(const std::string& name);

    // Appelé lorsque l'état du backend change
    void onBackendStateChanged(int state);

private:
    // Pointeur vers le gestionnaire d'affichage
    std::shared_ptr<displaymanager::DisplayManager> m_displayManager;

    // Pointeur vers la fenêtre principale
    Gtk::Window* m_mainWindow;

    // Gestionnaire de curseur
    std::shared_ptr<cursor_manager::CursorManager> m_cursorManager;

    // Backend actif (X11 ou Wayland)
    std::unique_ptr<InputBackend> m_backend;

    // État actuel
    bool m_initialized;

    // Méthodes utilitaires
    bool parseAccelerator(const std::string& accel, guint& key, Gdk::ModifierType& mods) const;

    // Gestionnaire d'événements clavier
    bool onKeyPressed(GdkEventKey* event);
};

}  // namespace input

#endif  // INPUT_MANAGER_H
