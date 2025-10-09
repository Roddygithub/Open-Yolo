#include "../include/config/ConfigManager.hpp"
#include "../include/log/Logger.hpp"

// Définition des variables globales
openyolo::config::ConfigManager g_config;
std::shared_ptr<openyolo::log::Logger> g_logger;

// Initialisation du logger
auto _ = []() {
    g_logger = std::make_shared<openyolo::log::Logger>();
    return 0;
}();
