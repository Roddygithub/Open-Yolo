// Standard C++
#include <algorithm>   // Pour std::find_if, etc.
#include <cstdint>     // Pour les types entiers
#include <filesystem>  // Pour la manipulation des chemins
#include <fstream>     // Pour la lecture/écriture de fichiers
#include <iomanip>     // Pour le formatage des sorties
#include <iostream>    // Pour std::cout, std::cerr
#include <map>         // Pour std::map
#include <memory>      // Pour std::shared_ptr, etc.
#include <sstream>     // Pour std::ostringstream
#include <stdexcept>   // Pour les exceptions standard
#include <string>      // Pour std::string
#include <utility>     // Pour std::move, std::pair
#include <vector>      // Pour std::vector

// GTKmm
#include <cairomm/context.h>  // Pour le rendu graphique
#include <gdkmm.h>            // Pour les fonctionnalités de bas niveau de GDK
#include <glibmm.h>           // Pour les utilitaires GLib
#include <gtkmm.h>            // Pour les widgets GTK

// Project headers
#include "../../include/cursormanager/CursorManager.hpp"
#include "../../include/displaymanager/DisplayManager.hpp"
#include "../../include/gui/MainWindow.hpp"
#include "../../include/input/InputManager.hpp"

// Alias for std::filesystem
namespace fs = std::filesystem;

namespace gui {

// MainWindow implementation
MainWindow::MainWindow(std::shared_ptr<cursor_manager::CursorManager> cursorManager,
                       std::shared_ptr<input::DisplayManager> displayManager,
                       std::shared_ptr<input::InputManager> inputManager)
    : mainBox_(Gtk::ORIENTATION_VERTICAL, 5),
      cursorManager_(std::move(cursorManager)),
      displayManager_(std::move(displayManager)),
      inputManager_(std::move(inputManager)) {
    // Configure the window
    set_title("Open-Yolo - Cursor Manager");
    set_default_size(800, 600);

    // Setup the UI
    setupWidgets();
    setupStatusBar();
    connectSignals();

    show_all_children();
}

MainWindow::~MainWindow() = default;

void MainWindow::addDefaultShortcuts() {
    // Implementation to be added
}

void MainWindow::setupWidgets() {
    // Create main container
    add(mainBox_);

    // Create cursor preview area
    cursorPreview_.set_size_request(200, 200);

    // Add widgets to main container
    mainBox_.pack_start(cursorPreview_, Gtk::PACK_EXPAND_WIDGET);
    mainBox_.pack_end(statusBar_, Gtk::PACK_SHRINK);
}

void MainWindow::setupStatusBar() { statusBar_.push("Ready"); }

void MainWindow::connectSignals() {
    cursorPreview_.signal_draw().connect(sigc::mem_fun(*this, &MainWindow::onCursorPreviewDraw));
}

void MainWindow::updateCursorPreview() { cursorPreview_.queue_draw(); }

void MainWindow::onCursorSelected() { updateCursorPreview(); }

bool MainWindow::onCursorPreviewDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
    Gtk::Allocation allocation = cursorPreview_.get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_source_rgb(1.0, 1.0, 1.0);  // White background
    cr->paint();

    if (cursorManager_) {
        cursorManager_->draw(cr, width / 2, height / 2, 1.0);
    } else {
        cr->set_source_rgb(0.5, 0.5, 0.5);
        cr->move_to(10, 20);
        cr->show_text("No cursor available");
    }

    return true;
}

}  // namespace gui
