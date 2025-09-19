#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <gtkmm/window.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/scale.h>
#include <gtkmm/switch.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/notebook.h>
#include <memory>
#include "cursormanager/CursorManager.hpp"
#include "displaymanager/DisplayManager.hpp"
#include "input/InputManager.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow(
        std::shared_ptr<cursor_manager::CursorManager> cursorManager,
        std::shared_ptr<DisplayManager> displayManager,
        std::shared_ptr<InputManager> inputManager
    );
    
    virtual ~MainWindow();
    
    // Méthodes pour mettre à jour l'interface utilisateur
    void updateCursorList();
    void updateEffectSettings();
    void updateDisplaySettings();
    
    // Méthodes d'aide
    void setupShortcuts();
    void addDefaultShortcuts();
    
protected:
    // Gestionnaires de signaux
    void onCursorSelected();
    void onScaleChanged();
    void onEffectToggled();
    void onApplyClicked();
    void onAboutClicked();
    
    // Méthodes d'aide
    
private:
    // Références aux gestionnaires
    std::shared_ptr<cursor_manager::CursorManager> cursorManager_;
    std::shared_ptr<DisplayManager> displayManager_;
    std::shared_ptr<InputManager> inputManager_;
    
    // Widgets de l'interface utilisateur
    Glib::RefPtr<Gtk::Builder> builder_;
    
    // Conteneurs principaux
    Gtk::Box* mainBox_;
    Gtk::Notebook* notebook_;
    
    // Onglet Curseur
    Gtk::ComboBoxText* cursorCombo_;
    Gtk::Scale* scaleX_;
    Gtk::Scale* scaleY_;
    Gtk::Switch* enableEffectsSwitch_;
    Gtk::Button* applyButton_;
    
    // Onglet Effets
    Gtk::CheckButton* shadowEffectCheck_;
    Gtk::Scale* shadowOpacityScale_;
    Gtk::Scale* shadowBlurScale_;
    Gtk::ColorButton* shadowColorButton_;
    
    // Onglet Affichage
    Gtk::ComboBoxText* displayCombo_;
    Gtk::Switch* followMouseSwitch_;
    
    // Onglet Raccourcis
    // (À implémenter)
    
    // État de l'application
    bool updatingUI_;
};

#endif // MAIN_WINDOW_HPP
