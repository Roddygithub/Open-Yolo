#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/window.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>  // For GDK_WINDOWING_X11
#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include <csignal>
#include <filesystem>
#include <cstdlib>

// Inclure nos en-têtes
#include "../include/config/ConfigManager.hpp"
#include "../include/log/Logger.hpp"
#include "../include/cursormanager/CursorManager.hpp"
#include "../include/cursormanager/ShaderProgram.hpp"
#include "../include/gui/MainWindow.hpp"
#include "../include/input/InputManager.hpp"
#include "../include/displaymanager/DisplayManager.hpp"

// Alias pour faciliter l'accès
namespace cfg = openyolo::config;
namespace log = openyolo::log;
namespace cm = cursor_manager;

// Global flag for clean shutdown
std::atomic<bool> g_running{true};

// Configuration et logger globaux
std::shared_ptr<cfg::ConfigManager> g_config;
std::shared_ptr<log::Logger> g_logger;

// Signal handler for clean shutdown
void signalHandler(int signum) {
    LOG_INFO("Signal de terminaison reçu, arrêt en cours...");
    g_running = false;
}

// Initialise le système de configuration
void initializeConfig() {
    try {
        // Obtenir le chemin de configuration
        auto configPath = cfg::ConfigManager::getDefaultConfigPath();
        
        // Créer le gestionnaire de configuration
        g_config = std::make_shared<cfg::ConfigManager>();
        
        // Charger la configuration si le fichier existe
        if (std::filesystem::exists(configPath)) {
            g_config->load(configPath);
            LOG_INFO("Configuration chargée depuis " + configPath.string());
        } else {
            // Créer une configuration par défaut
            LOG_INFO("Aucune configuration trouvée, utilisation des valeurs par défaut");
            
            // Définir les valeurs par défaut
            g_config->setValue("Application", "name", "Open-Yolo");
            g_config->setValue("Application", "version", "1.0.0");
            g_config->setValue("Application", "debug", false);
            g_config->setValue("Application", "log_level", 1);
            
            // Valeurs par défaut pour la fenêtre
            g_config->setValue("Window", "fullscreen", false);
            g_config->setValue("Window", "width", 1024);
            g_config->setValue("Window", "height", 768);
            
            // Valeurs par défaut pour le curseur
            g_config->setValue("Cursor", "enabled", true);
            g_config->setValue("Cursor", "size", 1.0f);
            
            // Sauvegarder la configuration par défaut
            g_config->save(configPath);
            LOG_INFO("Configuration par défaut enregistrée dans " + configPath.string());
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation de la configuration: " << e.what() << std::endl;
        throw;
    }
}

// Initialise le système de journalisation
void initializeLogger() {
    try {
        // Obtenir le niveau de log depuis la configuration
        int logLevel = g_config->getValue("Application", "log_level", 1);
        bool debugMode = g_config->getValue("Application", "debug", false);
        
        // Déterminer le niveau de log minimum
        log::LogLevel minLevel = log::LogLevel::INFO;
        if (debugMode) {
            minLevel = log::LogLevel::DEBUG;
        } else {
            switch (logLevel) {
                case 0: minLevel = log::LogLevel::ERROR; break;
                case 1: minLevel = log::LogLevel::WARNING; break;
                case 2: minLevel = log::LogLevel::INFO; break;
                case 3: minLevel = log::LogLevel::DEBUG; break;
                case 4: minLevel = log::LogLevel::TRACE; break;
                default: minLevel = log::LogLevel::INFO;
            }
        }
        
        // Obtenir le répertoire de logs depuis la configuration ou utiliser la valeur par défaut
        std::string logDir = g_config->getValue("Paths", "log_dir", "");
        if (logDir.empty()) {
            // Utiliser le répertoire par défaut
            logDir = (std::filesystem::path(g_config->getValue("Paths", "config_dir", "")) / "logs").string();
        }
        
        // Initialiser le logger
        log::Logger::instance().initialize(
            logDir,                            // Répertoire des logs
            5 * 1024 * 1024,                  // 5 Mo par fichier
            10,                               // 10 fichiers maximum
            minLevel,                         // Niveau de log minimum
            true                              // Afficher dans la console
        );
        
        LOG_INFO("Système de journalisation initialisé (niveau: " + std::to_string(static_cast<int>(minLevel)) + ")");
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation du système de journalisation: " << e.what() << std::endl;
        throw;
    }
}

// Nettoie les ressources à la fermeture de l'application
void cleanup() {
    LOG_INFO("Nettoyage des ressources...");
    
    // Sauvegarder la configuration si nécessaire
    if (g_config) {
        try {
            g_config->save(cfg::ConfigManager::getDefaultConfigPath());
            LOG_INFO("Configuration sauvegardée");
        } catch (const std::exception& e) {
            LOG_ERROR("Erreur lors de la sauvegarde de la configuration: " + std::string(e.what()));
        }
    }
    
    // Fermer le logger
    log::Logger::instance().shutdown();
    
    LOG_INFO("Application arrêtée");
}

// Initialize GTK environment
void initializeGtkEnvironment() {
    // Set GTK environment variables
    g_setenv("GTK_THEME", "Adwaita", TRUE);
    g_setenv("GTK_THEME_VARIANT", "light", TRUE);
    g_setenv("GDK_BACKEND", "x11", TRUE);
    g_setenv("NO_AT_BRIDGE", "1", TRUE);
    g_setenv("GTK_OVERLAY_SCROLLING", "0", TRUE);
    g_setenv("GTK_CSD", "0", TRUE);
    
    // Disable accessibility
    g_setenv("NO_AT_BRIDGE", "1", TRUE);
    
    // Disable client-side decorations
    g_setenv("GTK_CSD", "0", TRUE);
    
    // Disable overlay scrollbars
    g_setenv("GTK_OVERLAY_SCROLLING", "0", TRUE);
    
    // Disable animations
    g_setenv("GTK_DEBUG", "no-cursor-blink,no-cursor-blink-timeout", TRUE);
    
    // Disable XInput2 to avoid issues with Xvfb
    g_setenv("GDK_CORE_DEVICE_EVENTS", "1", TRUE);
    
    // Disable client-side decorations
    g_setenv("GTK_CSD", "0", TRUE);
    
    // Disable overlay scrollbars
    g_setenv("GTK_OVERLAY_SCROLLING", "0", TRUE);
    
    // Force X11 backend
    gdk_set_allowed_backends("x11");
    
    // Disable multidevice support
    gdk_disable_multidevice();
    
    // Set up signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

// Fonction pour la boucle de rendu dans un thread séparé
void renderLoop(std::shared_ptr<cm::CursorManager> cursorManager, 
                std::shared_ptr<DisplayManager> displayManager,
                std::atomic<bool>& running) {
    try {
        LOG_DEBUG("Démarrage de la boucle de rendu");
        
        while (running) {
            try {
                // Mettre à jour le rendu
                if (displayManager && cursorManager) {
                    displayManager->beginFrame();
                    cursorManager->render();
                    displayManager->endFrame();
                }
                
                // Limitation à environ 60 FPS
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            } catch (const std::exception& e) {
                LOG_ERROR("Erreur dans la boucle de rendu: " + std::string(e.what()));
                // Pause pour éviter une boucle d'erreur trop rapide
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        
        LOG_DEBUG("Arrêt de la boucle de rendu");
    } catch (const std::exception& e) {
        LOG_FATAL("Erreur fatale dans la boucle de rendu: " + std::string(e.what()));
    }
}

int main(int argc, char* argv[]) {
    // Initialiser le système de configuration
    try {
        initializeConfig();
    } catch (const std::exception& e) {
        std::cerr << "Échec de l'initialisation de la configuration: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // Initialiser le système de journalisation
    try {
        initializeLogger();
    } catch (const std::exception& e) {
        std::cerr << "Échec de l'initialisation du système de journalisation: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // Enregistrer le gestionnaire de nettoyage
    std::atexit(cleanup);
    
    // Configurer les gestionnaires de signaux
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    
    LOG_INFO("Démarrage de l'application Open-Yolo");
    LOG_DEBUG("Arguments de la ligne de commande (" + std::to_string(argc) + "):");
    for (int i = 0; i < argc; ++i) {
        LOG_DEBUG("  [" + std::to_string(i) + "]: " + (argv[i] ? argv[i] : "null"));
    }

    // Initialiser l'environnement GTK
    try {
        LOG_INFO("Initialisation de l'environnement GTK...");
        initializeGtkEnvironment();
    } catch (const std::exception& e) {
        LOG_FATAL("Échec de l'initialisation de l'environnement GTK: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    // Créer l'application GTK
    LOG_DEBUG("Création de l'application GTK...");
    auto app = Gtk::Application::create(argc, argv, "org.openyolo.app");

    // Créer les gestionnaires avec la configuration
    LOG_INFO("Création des gestionnaires...");
    
    // Créer le gestionnaire d'affichage avec la configuration
    int width = g_config->getValue("Window", "width", 1024);
    int height = g_config->getValue("Window", "height", 768);
    bool fullscreen = g_config->getValue("Window", "fullscreen", false);
    
    LOG_DEBUG("Création du gestionnaire d'affichage...");
    auto displayManager = std::make_shared<DisplayManager>();
    
    // Configurer le gestionnaire d'affichage
    displayManager->setWindowSize(width, height);
    displayManager->setFullscreen(fullscreen);
    
    // Créer le gestionnaire de curseur avec la configuration
    bool customCursors = g_config->getValue("Cursor", "enabled", true);
    float cursorSize = g_config->getValue("Cursor", "size", 1.0f);
    
    LOG_DEBUG("Création du gestionnaire de curseur...");
    auto cursorManager = std::make_shared<cm::CursorManager>();
    
    // Configurer le gestionnaire de curseur
    cursorManager->setEnabled(customCursors);
    cursorManager->setSize(cursorSize);
    
    // Créer le gestionnaire d'entrée
    LOG_DEBUG("Création du gestionnaire d'entrée...");
    auto inputManager = std::make_shared<InputManager>();

    // Initialiser les gestionnaires
    try {
        LOG_INFO("Initialisation du gestionnaire d'affichage...");
        if (!displayManager->initialize()) {
            throw std::runtime_error("Échec de l'initialisation du gestionnaire d'affichage");
        }

        LOG_INFO("Initialisation du gestionnaire de curseur...");
        if (!cursorManager->initialize()) {
            throw std::runtime_error("Échec de l'initialisation du gestionnaire de curseur");
        }
        
        // Configurer le gestionnaire d'entrée après l'initialisation des autres gestionnaires
        LOG_DEBUG("Configuration du gestionnaire d'entrée...");
        if (!inputManager->initialize()) {
            LOG_WARNING("Échec de l'initialisation du gestionnaire d'entrée, certaines fonctionnalités pourraient être limitées");
        }
    } catch (const std::exception& e) {
        LOG_FATAL("Erreur lors de l'initialisation des gestionnaires: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    // Créer et configurer la fenêtre principale
    std::unique_ptr<MainWindow> mainWindow;
    
    try {
        LOG_INFO("Création de la fenêtre principale...");
        mainWindow = std::make_unique<MainWindow>(cursorManager, displayManager, inputManager);

        // Configurer l'intégration GTK après la création de la fenêtre
        LOG_DEBUG("Configuration de l'intégration GTK...");
        if (!inputManager->setupGTKIntegration(mainWindow.get(), cursorManager)) {
            LOG_WARNING("L'intégration GTK n'a pas pu être configurée correctement");
        }

        // Ajouter les raccourcis par défaut
        LOG_DEBUG("Ajout des raccourcis par défaut...");
        mainWindow->addDefaultShortcuts();
        
    } catch (const std::exception& e) {
        LOG_FATAL("Erreur lors de la création de la fenêtre principale: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    // Démarrer la boucle de rendu dans un thread séparé
    LOG_INFO("Démarrage de la boucle de rendu...");
    std::thread renderThread(renderLoop, cursorManager, displayManager, std::ref(g_running));

    // Exécuter la boucle principale de l'application
    int result = EXIT_SUCCESS;
    try {
        LOG_INFO("Démarrage de la boucle principale de l'application...");
        result = app->run(*mainWindow);
    } catch (const std::exception& e) {
        LOG_FATAL("Erreur dans la boucle principale: " + std::string(e.what()));
        result = EXIT_FAILURE;
    }

    // Demander l'arrêt du thread de rendu
    LOG_INFO("Arrêt de la boucle de rendu...");
    g_running = false;

    // Attendre que le thread de rendu se termine
    if (renderThread.joinable()) {
        renderThread.join();
        LOG_DEBUG("Thread de rendu arrêté");
    }
    
    // Nettoyer les ressources
    try {
        LOG_DEBUG("Nettoyage des ressources...");
        mainWindow.reset();
        inputManager.reset();
        cursorManager.reset();
        displayManager.reset();
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du nettoyage des ressources: " + std::string(e.what()));
    }

    return result;
}
