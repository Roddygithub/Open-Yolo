#include "openyolo/AppCore.hpp"

#include <gtkmm.h>

#include <csignal>
#include <iostream>

#include "config/ConfigManager.hpp"
#include "log/Logger.hpp"

// Déclaration des variables globales
std::shared_ptr<cursor_manager::CursorManager> g_cursorManager;
std::shared_ptr<input::DisplayManager> g_displayManager;

// Définition des variables statiques
namespace openyolo {
std::atomic<bool> AppState::running{false};
Glib::RefPtr<Gtk::Application> AppState::app;
Gtk::Window* AppState::mainWindow = nullptr;
}  // namespace openyolo

namespace openyolo {

void handleError(const std::string& message, const std::exception* e) {
    try {
        if (e) {
            std::cerr << "Erreur: " << message << std::endl;
            std::cerr << "Détails: " << e->what() << std::endl;
        } else {
            std::cerr << "Erreur: " << message << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Erreur lors de la gestion d'une erreur: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Erreur inconnue lors de la gestion d'une erreur" << std::endl;
    }
}

void cleanup() {
    try {
        std::cout << "Nettoyage des ressources..." << std::endl;

        // Arrêter le thread de rendu si nécessaire
        app_state::running = false;

        // Sauvegarder la configuration
        try {
            auto& config = openyolo::ConfigManager::instance();
            config.save(config.getDefaultConfigPath());
            std::cout << "Configuration sauvegardée" << std::endl;
        } catch (const std::exception& e) {
            std::string errorMsg =
                "Erreur lors de la sauvegarde de la configuration: " + std::string(e.what());
            std::cerr << errorMsg << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du nettoyage: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Erreur inconnue lors du nettoyage" << std::endl;
    }
}

void signalHandler(int signum) {
    std::cout << "Signal reçu: " << signum << std::endl;

    switch (signum) {
        case SIGINT:   // Interruption (Ctrl+C)
        case SIGTERM:  // Terminaison
            std::cout << "Arrêt de l'application..." << std::endl;
            app_state::running = false;
            if (app_state::app) {
                app_state::app->quit();
            }
            break;

        case SIGSEGV:  // Erreur de segmentation
        case SIGABRT:  // Abandon
            std::cerr << "Erreur critique détectée. Arrêt de l'application." << std::endl;
            cleanup();
            std::signal(signum, SIG_DFL);
            std::raise(signum);
            break;

        default:
            break;
    }
}

int initialize(int argc, char* argv[]) {
    // Initialisation du logger en premier
    try {
        std::cout << "Initialisation du système de journalisation..." << std::endl;
        // Ici, vous devriez initialiser votre système de journalisation
    } catch (const std::exception& e) {
        std::cerr << "Échec de l'initialisation du système de journalisation: " << e.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Configuration des gestionnaires de signaux
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);

    // Enregistrer le gestionnaire de nettoyage
    std::atexit(cleanup);

    std::cout << "Démarrage de l'application Open-Yolo" << std::endl;

    return 0;  // Succès
}

}  // namespace openyolo
