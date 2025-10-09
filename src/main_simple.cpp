#include <gtkmm/application.h>
#include <gtkmm/window.h>

#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

// En-têtes du projet
#include "config/ConfigManager.hpp"
#include "cursormanager/CursorManager.hpp"
#include "displaymanager/DisplayManager.hpp"
#include "input/InputManager.hpp"
#include "openyolo/AppCore.hpp"

// Définition des macros de journalisation
#define OY_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#define OY_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define OY_WARNING(msg) std::cerr << "[WARNING] " << msg << std::endl
#define OY_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define OY_FATAL(msg)                                \
    {                                                \
        std::cerr << "[FATAL] " << msg << std::endl; \
        std::exit(EXIT_FAILURE);                     \
    }

// Alias pour les espaces de noms
namespace cursor_manager = ::cursor_manager;
namespace input = ::input;

// Variables globales
static std::shared_ptr<cursor_manager::CursorManager> g_cursorManager;
static std::shared_ptr<input::DisplayManager> g_displayManager;
static std::shared_ptr<input::InputManager> g_inputManager;
static std::shared_ptr<std::thread> g_renderThread;
static std::atomic<bool> g_running{false};

// Fonction utilitaire pour lire les valeurs de configuration
template <typename T>
T getConfigValue(const std::string& section, const std::string& key, const T& defaultValue) {
    try {
        return openyolo::ConfigManager::instance().getValue(section, key, defaultValue);
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] Erreur de configuration [" << section << "] " << key << ": "
                  << e.what() << std::endl;
        return defaultValue;
    }
}

// Fonction de rendu dans un thread séparé
void renderLoop() {
    try {
        OY_INFO("Démarrage de la boucle de rendu...");

        while (g_running && g_cursorManager) {
            // Mise à jour du curseur
            // Note: La méthode update() n'existe pas dans CursorManager, nous l'utilisons comme
            // exemple Si vous avez besoin d'une méthode de mise à jour, vous devrez l'implémenter
            // dans CursorManager Mettre à jour le rendu ici
            if (g_cursorManager) {
                g_cursorManager->update();
            }

            // Petite pause pour éviter une utilisation élevée du CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
        }

    } catch (const std::exception& e) {
        OY_ERROR("Erreur dans la boucle de rendu: " + std::string(e.what()));
    }
}

// Gestionnaire de signaux
void signalHandler(int signum) {
    OY_INFO("Signal reçu: " + std::to_string(signum));
    g_running = false;

    // Si nous avons une application GTK, la quitter
    if (openyolo::app_state::app) {
        openyolo::app_state::app->quit();
    }
}

int main(int argc, char* argv[]) {
    try {
        // Configuration des gestionnaires de signaux
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // Initialisation de base via AppCore
        int result = openyolo::initialize(argc, argv);
        if (result != 0) {
            return result;
        }

        // Initialisation des gestionnaires
        try {
            OY_INFO("Initialisation des gestionnaires...");

            // Créer les gestionnaires
            g_displayManager = std::make_shared<input::DisplayManager>();
            g_cursorManager = std::make_shared<cursor_manager::CursorManager>();
            g_inputManager = std::make_shared<input::InputManager>();

            // Configuration initiale
            auto& config = openyolo::ConfigManager::instance();

            // Charger la configuration
            try {
                config.load(config.getDefaultConfigPath());
            } catch (const std::exception& e) {
                OY_WARNING("Impossible de charger la configuration: " + std::string(e.what()));
                // Continuer avec les valeurs par défaut
            }

            // Créer la fenêtre principale
            if (!openyolo::app_state::mainWindow) {
                openyolo::app_state::mainWindow = Gtk::make_managed<Gtk::Window>();
                openyolo::app_state::mainWindow->set_title("Open-Yolo");
                openyolo::app_state::mainWindow->set_default_size(800, 600);
            }

            // Démarrer le thread de rendu
            g_running = true;
            g_renderThread = std::make_shared<std::thread>(renderLoop);

        } catch (const std::exception& e) {
            OY_ERROR("Échec de l'initialisation: " + std::string(e.what()));
            return EXIT_FAILURE;
        }

        // Boucle principale de l'application
        OY_INFO("Open-Yolo démarré avec succès");

        // Lancer la boucle principale GTK
        int appResult = openyolo::app_state::app->run(*openyolo::app_state::mainWindow);

        // Nettoyage
        g_running = false;

        // Attendre que le thread de rendu se termine
        if (g_renderThread && g_renderThread->joinable()) {
            g_renderThread->join();
        }

        return appResult;

    } catch (const std::exception& e) {
        OY_FATAL("Erreur critique: " + std::string(e.what()));
    } catch (...) {
        OY_FATAL("Erreur inconnue");
    }

    return EXIT_FAILURE;
}
