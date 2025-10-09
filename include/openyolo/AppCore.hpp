#ifndef OPENYOLO_APP_CORE_HPP
#define OPENYOLO_APP_CORE_HPP

#include <atomic>
#include <csignal>
#include <memory>
#include <string>

// En-têtes GTK
#include <gtkmm/application.h>
#include <gtkmm/window.h>

// Forward declarations
namespace cursor_manager {
class CursorManager;
}

namespace input {
class DisplayManager;
}

namespace openyolo {

// Structure pour l'état de l'application
struct AppState {
    static std::atomic<bool> running;
    static Glib::RefPtr<Gtk::Application> app;
    static Gtk::Window* mainWindow;
};

// Alias pour la compatibilité
using app_state = AppState;

// Déclaration des fonctions principales
void handleError(const std::string& message, const std::exception* e = nullptr);
void cleanup();
void signalHandler(int signum);
void initializeConfig();
void initializeDefaultConfig();
void initializeLogger();
void initializeGtkEnvironment();
void renderLoop(std::shared_ptr<cursor_manager::CursorManager> cursorManager,
                std::shared_ptr<input::DisplayManager> displayManager);
int initialize(int argc, char* argv[]);

}  // namespace openyolo

// Déclaration de la fonction main globale
int main(int argc, char* argv[]);

#endif  // OPENYOLO_APP_CORE_HPP
