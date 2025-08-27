#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/window.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include "cursormanager/CursorManager.hpp"
#include "displaymanager/DisplayManager.hpp"
#include "input/InputManager.hpp"
#include "gui/MainWindow.hpp"

// Déclaration anticipée pour InputManager
class InputManager;

// Alias pour les namespaces
namespace cm = cursor_manager;

// Fonction pour la boucle de rendu dans un thread séparé
void renderLoop(std::shared_ptr<cm::CursorManager> cursorManager, 
                std::shared_ptr<DisplayManager> displayManager,
                std::atomic<bool>& running) {
    try {
        while (running) {
            // Mise à jour des écrans et du curseur
            if (displayManager) {
                displayManager->updateDisplays();
            }
            if (cursorManager) {
                cursorManager->update();
            }
            
            // Limitation à environ 60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur dans la boucle de rendu: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Initialisation de GTK avec les bons paramètres pour GTK4
    auto app = Gtk::Application::create("org.openyolo.app");
    
    try {
        // Initialisation des gestionnaires
        auto cursorManager = std::make_shared<cm::CursorManager>();
        if (!cursorManager) {
            throw std::runtime_error("Échec de la création du gestionnaire de curseur");
        }
        
        auto displayManager = std::make_shared<DisplayManager>();
        if (!displayManager) {
            throw std::runtime_error("Échec de la création du gestionnaire d'affichage");
        }
        
        // Création et initialisation du gestionnaire d'entrée
        auto inputManager = std::make_shared<InputManager>();
        if (!inputManager) {
            throw std::runtime_error("Échec de la création du gestionnaire d'entrée");
        }
        
        // Configuration de l'intégration GTK
        inputManager->setupGTKIntegration(nullptr, cursorManager);
        
        // Initialisation des gestionnaires
        if (!cursorManager->initialize()) {
            throw std::runtime_error("Échec de l'initialisation du gestionnaire de curseur");
        }
        
        if (!displayManager->initialize()) {
            throw std::runtime_error("Échec de l'initialisation du gestionnaire d'affichage");
        }
        
        // Création de la fenêtre principale
        auto mainWindow = std::make_unique<MainWindow>(cursorManager, displayManager, inputManager);
        
        // Démarrer la boucle de rendu dans un thread séparé
        std::atomic<bool> running{true};
        std::thread renderThread(renderLoop, cursorManager, displayManager, std::ref(running));
        
        // Configuration du style CSS
        auto cssProvider = Gtk::CssProvider::create();
        cssProvider->load_from_data(
            "window { background-color: #f5f5f5; color: #333333; }"
            "button { background-color: #4a90e2; color: white; border-radius: 4px; padding: 6px 12px; }"
            "button:hover { background-color: #357abd; }"
            "label { color: #333333; }"
            ".error { color: #e74c3c; font-weight: bold; }"
        );
        
        // Application du style CSS
        auto display = Gdk::Display::get_default();
        if (display) {
            // Pour GTKMM 3.0, on utilise Gtk::StyleContext::add_provider_for_screen
            auto screen = display->get_default_screen();
            if (screen) {
                // Utilisation de la méthode gtk_style_context_add_provider_for_screen directement
                // avec les pointeurs GObject sous-jacents
                gtk_style_context_add_provider_for_screen(
                    screen->gobj(),
                    GTK_STYLE_PROVIDER(cssProvider->gobj()),
                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
                );
            }
        }
        
        // Configuration du gestionnaire de fermeture pour GTK3
        mainWindow->signal_delete_event().connect([&](GdkEventAny* event) -> bool {
            running = false;
            if (renderThread.joinable()) {
                renderThread.join();
            }
            return false; // Autoriser la fermeture
        });
        
        // Afficher la fenêtre principale
        mainWindow->show();
        
        // Démarrer la boucle d'événements GTK
        int result = app->run();
        
        // Nettoyage
        running = false;
        if (renderThread.joinable()) {
            renderThread.join();
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
}
