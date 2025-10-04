#pragma once

// Standard C++
#include <memory>
#include <string>

// GTKmm
#include <gtkmm.h>
#include <cairomm/context.h>

// Forward declarations
namespace cursor_manager {
    class CursorManager;
}

namespace input {
    class DisplayManager;
    class InputManager;
}

namespace gui {
    
    class MainWindow : public Gtk::Window {
    public:
        MainWindow(
            std::shared_ptr<cursor_manager::CursorManager> cursorManager,
            std::shared_ptr<::input::DisplayManager> displayManager,
            std::shared_ptr<::input::InputManager> inputManager
        );
        virtual ~MainWindow();

        void addDefaultShortcuts();

    private:
        // UI setup methods
        void setupWidgets();
        void setupStatusBar();
        void connectSignals();
        void updateCursorPreview();

        // Event handlers
        void onCursorSelected();
        bool onCursorPreviewDraw(const Cairo::RefPtr<Cairo::Context>& cr);

        // Member variables
        Gtk::Box mainBox_;
        Gtk::DrawingArea cursorPreview_;
        Gtk::Statusbar statusBar_;
        std::shared_ptr<cursor_manager::CursorManager> cursorManager_;
        std::shared_ptr<input::DisplayManager> displayManager_;
        std::shared_ptr<input::InputManager> inputManager_;
    };
} // namespace gui
