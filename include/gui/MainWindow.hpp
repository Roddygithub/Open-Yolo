#pragma once

#include <gtkmm.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <gdk/gdk.h> // Pour gtk_accelerator_parse
#include "../input/InputManager.hpp"

// Forward declarations
namespace cursor_manager {
    class CursorManager;
}
class DisplayManager;

// Structure pour les informations de raccourci
struct ShortcutInfo {
    std::string name;
    std::string description;
    std::string defaultAccelerator;
    std::function<void()> callback;
};

class MainWindow : public Gtk::Window {
public:
    MainWindow(
        std::shared_ptr<cursor_manager::CursorManager> cursorManager,
        std::shared_ptr<DisplayManager> displayManager,
        std::shared_ptr<InputManager> inputManager
    );
    virtual ~MainWindow();

    // Gestion des raccourcis
    void registerShortcut(const ShortcutInfo& shortcut);
    void updateShortcut(const std::string& name, const std::string& newAccelerator);
    
    // Configuration de l'interface
    void setupWidgets();
    
    // Mise à jour de l'interface
    void updateDisplayList();
    void updateCursorPreview();
    
    // Affichage des messages
    void showStatus(const std::string& message, bool isError = false);
    void showError(const std::string& message);
    
    // Gestion des effets
    void setAnimationSpeed(int fps);
    
    // Affichage de la boîte de dialogue À propos
    void showAboutDialog();
    
    // Ajout des raccourcis par défaut
    void addDefaultShortcuts();

protected:
    // Gestionnaires d'événements
    bool on_delete_event(GdkEventAny* event) override;
    
    // Méthodes de rendu personnalisé
    void on_shortcut_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
    void on_scale_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
    
    // Gestionnaires de signaux
    void on_cursor_file_selected();
    void on_fps_changed();
    void on_scale_changed();
    void on_shortcut_edited(const Glib::ustring& path_string, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode);
    void on_shortcut_triggered(const std::string& name);
    void on_display_settings_changed();
    void on_apply_clicked();
    void on_cancel_clicked();
    void on_about_activated();
    void show_license_dialog();

private:
    // Méthodes d'initialisation
    void setupCursorTab(Gtk::Box* parent);
    void setupShortcutsTab(Gtk::Box* parent);
    void setupDisplayTab(Gtk::Box* parent);
    void setupAboutTab(Gtk::Box* parent);
    void setupStatusBar();
    void connectSignals();
    
    // Méthodes utilitaires
    void applySettings();
    void loadSettings();
    void saveSettings();
    
    // Gestion des signaux
    void onCursorSelected();
    void onApplyClicked();
    void onCancelClicked();
    void onFpsChanged();
    void onScaleChanged();
    void onDisplayChanged();
    void onShortcutEdited(const Glib::ustring& path, guint keyval, 
                         Gdk::ModifierType modifiers, guint hardware_keycode);
    void onCursorFileSelected();
    void onCursorSettingsChanged();
    void onDisplaySettingsChanged();
    void onDisplayToggled(const Glib::ustring& path);
    void onScaleCellData(Gtk::CellRenderer* cell, const Gtk::TreeModel::const_iterator& iter);
    void onShortcutCellData(Gtk::CellRenderer* cell, const Gtk::TreeModel::const_iterator& iter);
    
    // Gestion des raccourcis
    void onShortcutTriggered(const std::string& name);
    
    // Utilitaires d'interface
    void updateStatus(const std::string& message);
    void showLicenseDialog();

private:
    // Gestionnaires
    std::shared_ptr<cursor_manager::CursorManager> cursorManager_;
    std::shared_ptr<DisplayManager> displayManager_;
    std::shared_ptr<InputManager> inputManager_;
    
    // Widgets principaux
    Gtk::Box mainBox_;
    Gtk::Notebook* notebook_ = nullptr;
    Gtk::Statusbar statusBar_;
    
    // Données des onglets
    struct {
        Gtk::Button* fileChooserBtn = nullptr;
        Gtk::DrawingArea* previewArea = nullptr;
        Gtk::SpinButton* fpsSpin = nullptr;
        Gtk::Scale* scaleSlider = nullptr;
    } cursorTab_;
    
    struct {
        Gtk::TreeView* treeView = nullptr;
        Glib::RefPtr<Gtk::ListStore> listStore;
        
        struct Columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> name;
            Gtk::TreeModelColumn<Glib::ustring> description;
            Gtk::TreeModelColumn<Glib::ustring> accelerator;
            Gtk::TreeModelColumn<guint> keyval;
            Gtk::TreeModelColumn<Gdk::ModifierType> modifiers;
            
            Columns() {
                add(name);
                add(description);
                add(accelerator);
                add(keyval);
                add(modifiers);
            }
        } columns;
    } shortcutsTab_;
    
    struct {
        Gtk::TreeView* displayTreeView = nullptr;
        Glib::RefPtr<Gtk::ListStore> displayList;
        Gtk::CheckButton* followMouseCheck = nullptr;
        
        struct Columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> name;
            Gtk::TreeModelColumn<Glib::ustring> resolution;
            Gtk::TreeModelColumn<bool> isPrimary;
            Gtk::TreeModelColumn<double> scale;
            Gtk::TreeModelColumn<gpointer> displayPtr;
            
            Columns() {
                add(name);
                add(resolution);
                add(isPrimary);
                add(scale);
                add(displayPtr);
            }
        } columns;
    } displayTab_;
    
    // Fonction de rendu pour l'aperçu du curseur
    void onCursorPreviewDraw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    
    // État de l'application
    std::string currentCursorPath_;
    bool settingsModified_ = false;
};
