#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gdk/gdkwayland.h>
#include <gdk/gdk.h>
#include <iostream>
#include <memory>
#include "input/InputManager.hpp"
#include "input/backends/WaylandBackend.hpp"
#include "input/ErrorCodes.hpp"

using namespace input;

class TestWindow : public Gtk::Window {
public:
    TestWindow() {
        set_title("Test des raccourcis Wayland");
        set_default_size(400, 200);

        // Création de l'interface
        auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
        add(*vbox);

        // Zone d'affichage des messages
        m_status_label.set_text("Appuyez sur un raccourci...");
        m_status_label.set_halign(Gtk::ALIGN_CENTER);
        m_status_label.set_valign(Gtk::ALIGN_CENTER);
        vbox->pack_start(m_status_label, true, true, 0);

        // Bouton pour quitter
        auto quit_button = Gtk::make_managed<Gtk::Button>("Quitter");
        quit_button->signal_clicked().connect([this]() { close(); });
        vbox->pack_end(*quit_button, false, false, 10);

        // Initialisation du gestionnaire de raccourcis
        try {
            // Créer le gestionnaire d'entrée avec un DisplayManager nul pour les tests
            m_input_manager = std::make_unique<InputManager>(nullptr);
            
            // Configurer l'intégration GTK avec la fenêtre actuelle
            m_input_manager->setupGTKIntegration(this, nullptr);
            
            // Enregistrement des raccourcis de test
            register_test_shortcuts();
            
            m_status_label.set_text("Raccourcis enregistrés. Essayez :\nCtrl+Alt+1, Ctrl+Alt+2, Ctrl+Alt+3");
            
        } catch (const std::exception& e) {
            std::cerr << "Erreur d'initialisation : " << e.what() << std::endl;
            m_status_label.set_text("Erreur d'initialisation. Voir la console pour plus de détails.");
        }

        show_all_children();
    }

    ~TestWindow() override = default;

private:
    void register_test_shortcuts() {
        if (!m_input_manager) return;

        // Raccourci 1
        m_input_manager->registerShortcut("test1", "<Ctrl><Alt>1", [this]() {
            std::cout << "Raccourci 1 activé" << std::endl;
            m_status_label.set_text("Raccourci 1 activé !");
        });

        // Raccourci 2
        m_input_manager->registerShortcut("test2", "<Ctrl><Alt>2", [this]() {
            std::cout << "Raccourci 2 activé" << std::endl;
            m_status_label.set_text("Raccourci 2 activé !");
        });

        // Raccourci 3
        m_input_manager->registerShortcut("test3", "<Ctrl><Alt>3", [this]() {
            std::cout << "Raccourci 3 activé" << std::endl;
            m_status_label.set_text("Raccourci 3 activé !");
        });

        std::cout << "Raccourcis enregistrés avec succès" << std::endl;
    }

    std::unique_ptr<InputManager> m_input_manager;
    Gtk::Label m_status_label;
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.openyolo.test.wayland_shortcuts");
    
    // Vérifier si nous sommes sous Wayland
    auto display = gdk_display_get_default();
    if (!GDK_IS_WAYLAND_DISPLAY(display)) {
        std::cerr << "Ce test nécessite un environnement Wayland." << std::endl;
        return 1;
    }
    
    TestWindow window;
    return app->run(window);
}
