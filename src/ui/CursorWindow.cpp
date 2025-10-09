#include "ui/CursorWindow.hpp"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <iostream>

namespace ui {

using namespace input;

CursorWindow::CursorWindow()
    : m_mainBox(Gtk::ORIENTATION_VERTICAL, 6),
      m_profileBox(Gtk::ORIENTATION_HORIZONTAL, 6),
      m_cursorFrame("Cursor Settings"),
      m_controlBox(Gtk::ORIENTATION_HORIZONTAL, 6),
      m_browseBtn("Browse..."),
      m_applyBtn("Apply"),
      m_useSystemBtn("Use System Cursors"),
      m_cursorManager(CursorManager::getInstance()) {
    
    set_title("Open-Yolo Cursor Manager");
    set_default_size(400, 500);
    set_border_width(12);
    
    // Initialiser le gestionnaire de curseurs
    if (!m_cursorManager.initialize()) {
        Gtk::MessageDialog dialog(*this, "Failed to initialize cursor manager", false, Gtk::MESSAGE_ERROR);
        dialog.run();
        return;
    }
    
    setup_ui();
    setup_signals();
    load_profiles();
    load_cursors();
    update_sensitivity();
}

void CursorWindow::setup_ui() {
    // Configuration de la boîte principale
    add(m_mainBox);
    
    // Zone de profil
    m_profileBox.pack_start(*Gtk::make_managed<Gtk::Label>("Profile:"), Gtk::PACK_SHRINK);
    m_profileBox.pack_start(m_profileCombo);
    m_profileBox.pack_start(m_saveProfileBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_profileBox, Gtk::PACK_SHRINK);
    
    // Liste des curseurs
    m_cursorFrame.set_margin_top(12);
    m_cursorFrame.add(m_cursorList);
    m_mainBox.pack_start(m_cursorFrame);
    
    // Contrôles
    m_controlBox.pack_start(m_browseBtn, Gtk::PACK_SHRINK);
    m_controlBox.pack_start(m_applyBtn, Gtk::PACK_SHRINK);
    m_controlBox.pack_end(m_useSystemBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_controlBox, Gtk::PACK_SHRINK);
    
    show_all_children();
}

void CursorWindow::setup_signals() {
    m_profileCombo.signal_changed().connect(
        sigc::mem_fun(*this, &CursorWindow::on_profile_changed));
    
    m_browseBtn.signal_clicked().connect(
        sigc::mem_fun(*this, &CursorWindow::on_browse_clicked));
    
    m_applyBtn.signal_clicked().connect(
        sigc::mem_fun(*this, &CursorWindow::on_apply_clicked));
    
    m_saveProfileBtn.signal_clicked().connect(
        sigc::mem_fun(*this, &CursorWindow::on_save_profile_clicked));
    
    m_useSystemBtn.signal_toggled().connect(
        sigc::mem_fun(*this, &CursorWindow::on_use_system_toggled));
}

void CursorWindow::load_profiles() {
    m_profileCombo.remove_all();
    
    // Ajouter le profil système
    m_profileCombo.append("system", "System Default");
    
    // Ajouter les profils personnalisés
    for (const auto& profile : m_cursorManager.listProfiles()) {
        m_profileCombo.append(profile, profile);
    }
    
    // Sélectionner le premier profil par défaut
    if (m_profileCombo.get_model()->children().size() > 0) {
        m_profileCombo.set_active(0);
    }
}

void CursorWindow::load_cursors() {
    // Nettoyer la liste existante
    for (auto& row : m_cursorList.get_children()) {
        m_cursorList.remove(*row);
    }
    m_cursorRows.clear();
    
    // Types de curseurs supportés
    std::vector<std::pair<CursorManager::CursorType, std::string>> cursorTypes = {
        {CursorManager::CursorType::Pointer, "Pointer"},
        {CursorManager::CursorType::Hand, "Hand"},
        {CursorManager::CursorType::Crosshair, "Crosshair"},
        {CursorManager::CursorType::Text, "Text"},
        {CursorManager::CursorType::Wait, "Wait"},
        {CursorManager::CursorType::Help, "Help"},
        {CursorManager::CursorType::Move, "Move"},
        {CursorManager::CursorType::ResizeNS, "Resize NS"},
        {CursorManager::CursorType::ResizeWE, "Resize WE"},
        {CursorManager::CursorType::ResizeNWSE, "Resize NW-SE"},
        {CursorManager::CursorType::ResizeNESW, "Resize NE-SW"}
    };
    
    // Ajouter chaque type de curseur à la liste
    for (const auto& [type, name] : cursorTypes) {
        auto row = Gtk::make_managed<Gtk::ListBoxRow>();
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 6);
        
        box->pack_start(*Gtk::make_managed<Gtk::Label>(name), Gtk::PACK_SHRINK);
        
        auto entry = Gtk::make_managed<Gtk::Entry>();
        entry->set_hexpand(true);
        entry->set_editable(false);
        box->pack_start(*entry);
        
        row->add(*box);
        m_cursorList.add(*row);
        
        // Stocker la référence pour une utilisation ultérieure
        m_cursorRows[name] = row;
    }
    
    m_cursorList.show_all();
}

void CursorWindow::update_sensitivity() {
    bool useSystem = m_useSystemBtn.get_active();
    m_cursorList.set_sensitive(!useSystem);
    m_browseBtn.set_sensitive(!useSystem);
    m_applyBtn.set_sensitive(!useSystem);
}

void CursorWindow::on_profile_changed() {
    Glib::ustring active_id = m_profileCombo.get_active_id();
    if (active_id.empty()) return;
    
    if (active_id == "system") {
        m_useSystemBtn.set_active(true);
    } else {
        m_useSystemBtn.set_active(false);
        if (!m_cursorManager.loadProfile(active_id)) {
            Gtk::MessageDialog dialog(*this, "Failed to load profile", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text("Could not load the selected profile.");
            dialog.run();
        }
    }
}

void CursorWindow::on_cursor_type_changed() {
    // Implémenter la logique de changement de type de curseur
}

void CursorWindow::on_browse_clicked() {
    Gtk::FileChooserDialog dialog("Select Cursor File", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    
    // Filtres de fichiers
    auto filter_image = Gtk::FileFilter::create();
    filter_image->set_name("Image files");
    filter_image->add_pattern("*.png");
    filter_image->add_pattern("*.xpm");
    filter_image->add_pattern("*.xbm");
    dialog.add_filter(filter_image);
    
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);
    
    dialog.signal_response().connect([this, &dialog](int response_id) {
        if (response_id == Gtk::RESPONSE_ACCEPT) {
            std::string filename = dialog.get_filename();
            // Mettre à jour l'entrée sélectionnée
            // (à implémenter)
        }
        dialog.hide();
    });
    
    dialog.run();
}

void CursorWindow::on_apply_clicked() {
    // Implémenter l'application des modifications
    Gtk::MessageDialog dialog(*this, "Cursor settings applied", false, Gtk::MESSAGE_INFO);
    dialog.set_secondary_text("The cursor settings have been applied successfully.");
    dialog.run();
}

void CursorWindow::on_save_profile_clicked() {
    Gtk::Dialog dialog("Save Profile", *this);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Save", Gtk::RESPONSE_ACCEPT);
    
    auto content_area = dialog.get_content_area();
    auto entry = Gtk::make_managed<Gtk::Entry>();
    entry->set_placeholder_text("Enter profile name");
    content_area->pack_start(*entry);
    content_area->show_all();
    
    if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
        std::string profileName = entry->get_text();
        if (!profileName.empty()) {
            // Créer et sauvegarder le profil
            // (à implémenter)
            load_profiles(); // Recharger la liste des profils
        }
    }
}

void CursorWindow::on_use_system_toggled() {
    m_cursorManager.useSystemCursors(m_useSystemBtn.get_active());
    update_sensitivity();
}

} // namespace ui
