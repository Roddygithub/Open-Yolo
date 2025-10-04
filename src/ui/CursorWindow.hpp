#pragma once

#include <gtkmm.h>
#include <input/CursorManager.hpp>

namespace ui {

class CursorWindow : public Gtk::Window {
public:
    CursorWindow();
    virtual ~CursorWindow() = default;

protected:
    // Signal handlers
    void on_profile_changed();
    void on_cursor_type_changed();
    void on_browse_clicked();
    void on_apply_clicked();
    void on_save_profile_clicked();
    void on_use_system_toggled();

    // Widgets
    Gtk::Box m_mainBox;
    Gtk::Box m_profileBox;
    Gtk::ComboBoxText m_profileCombo;
    Gtk::Button m_saveProfileBtn;
    Gtk::Frame m_cursorFrame;
    Gtk::ListBox m_cursorList;
    Gtk::Box m_controlBox;
    Gtk::Button m_browseBtn;
    Gtk::Button m_applyBtn;
    Gtk::CheckButton m_useSystemBtn;

    // Data
    input::CursorManager& m_cursorManager;
    std::map<std::string, Gtk::ListBoxRow*> m_cursorRows;

    // Helper methods
    void setup_ui();
    void setup_signals();
    void load_profiles();
    void load_cursors();
    void update_sensitivity();
};

} // namespace ui
