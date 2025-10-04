#include <gtkmm/application.h>
#include <iostream>
#include <memory>
#include <atomic>

// Forward declarations
namespace openyolo {
    namespace config { class ConfigManager; }
    namespace log { class Logger; }
    namespace input { class InputManager; class DisplayManager; }
    namespace gui { class MainWindow; }
    namespace cursor_manager { class CursorManager; }
}

// Global variables
extern openyolo::config::ConfigManager g_config;
extern std::shared_ptr<openyolo::log::Logger> g_logger;
std::atomic<bool> g_running{true};

// Simple logging macros
#define LOG_INFO(msg)    std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg)   std::cerr << "[ERROR] " << msg << std::endl

// Signal handler for clean shutdown
void signalHandler(int) {
    LOG_INFO("Signal received, shutting down...");
    g_running = false;
}

int main(int argc, char* argv[]) {
    try {
        // Set up signal handlers
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // Initialize GTK application
        auto app = Gtk::Application::create("org.openyolo.app");
        
        LOG_INFO("Open-Yolo starting...");
        
        // Main application loop
        while (g_running) {
            // Process GTK events
            while (g_main_context_pending(nullptr)) {
                g_main_context_iteration(nullptr, false);
            }
            
            // Small delay to prevent high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        LOG_INFO("Open-Yolo shutting down...");
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: " << e.what());
        return 1;
    }
}
