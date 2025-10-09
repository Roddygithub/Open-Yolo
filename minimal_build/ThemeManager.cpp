#include "ThemeManager.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pwd.h>
#include "Logger.hpp"

namespace openyolo {

namespace fs = std::filesystem;

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

bool ThemeManager::loadThemes() {
    LOG_INFO("Chargement des thèmes de curseurs...");
    m_availableThemes.clear();
    
    // Charger les thèmes système
    for (const auto& path : m_themePaths) {
        if (fs::exists(path) && fs::is_directory(path)) {
            LOG_DEBUG("Recherche de thèmes dans: " + path);
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_directory(entry)) {
                    loadThemeFromDir(entry.path(), true);
                }
            }
        }
    }
    
    // Charger le thème actuel
    const char* currentTheme = getenv("XCURSOR_THEME");
    if (currentTheme) {
        for (const auto& theme : m_availableThemes) {
            if (theme.name == currentTheme) {
                m_currentTheme = &theme;
                LOG_INFO("Thème actuel: " + theme.name);
                break;
            }
        }
    }
    
    LOG_INFO(std::to_string(m_availableThemes.size()) + " thèmes chargés");
    return !m_availableThemes.empty();
}

bool ThemeManager::loadThemeFromDir(const std::string& dirPath, bool isSystemTheme) {
    try {
        // Vérifier si c'est un dossier de thème de curseurs valide
        if (!isCursorThemeDir(dirPath)) {
            return false;
        }
        
        ThemeInfo theme;
        theme.path = dirPath;
        theme.name = fs::path(dirPath).filename().string();
        theme.isSystemTheme = isSystemTheme;
        
        // Lire le fichier index.theme
        std::ifstream themeFile(dirPath + "/index.theme");
        if (themeFile.is_open()) {
            std::string line;
            while (std::getline(themeFile, line)) {
                if (line.rfind("Name=", 0) == 0) {
                    theme.name = line.substr(5);
                } else if (line.rfind("Comment=", 0) == 0) {
                    theme.comment = line.substr(8);
                }
            }
        }
        
        // Vérifier si le thème existe déjà
        for (const auto& existing : m_availableThemes) {
            if (existing.name == theme.name) {
                return false; // Thème déjà chargé
            }
        }
        
        // Vérifier s'il y a des curseurs dans le thème
        std::string cursorPath = dirPath + "/cursors";
        if (fs::exists(cursorPath) && fs::is_directory(cursorPath)) {
            m_availableThemes.push_back(theme);
            LOG_DEBUG("Thème chargé: " + theme.name + " (" + dirPath + ")");
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du chargement du thème " + dirPath + ": " + e.what());
    }
    
    return false;
}

bool ThemeManager::isCursorThemeDir(const std::string& dirPath) const {
    // Vérifier si le dossier contient un fichier index.theme
    if (!fs::exists(dirPath + "/index.theme")) {
        return false;
    }
    
    // Vérifier si le fichier index.theme contient la section [Icon Theme]
    std::ifstream themeFile(dirPath + "/index.theme");
    if (themeFile.is_open()) {
        std::string line;
        while (std::getline(themeFile, line)) {
            if (line == "[Icon Theme]") {
                return true;
            }
        }
    }
    
    return false;
}

bool ThemeManager::applyTheme(const std::string& themeName) {
    LOG_INFO("Application du thème: " + themeName);
    
    // Trouver le thème
    for (const auto& theme : m_availableThemes) {
        if (theme.name == themeName) {
            // Définir la variable d'environnement
            if (setenv("XCURSOR_THEME", themeName.c_str(), 1) != 0) {
                LOG_ERROR("Impossible de définir XCURSOR_THEME");
                return false;
            }
            
            // Mettre à jour le curseur avec xsetroot
            std::string cmd = "xsetroot -cursor_name left_ptr";
            if (system(cmd.c_str()) != 0) {
                LOG_WARNING("Échec de la mise à jour du curseur avec xsetroot");
            }
            
            m_currentTheme = &theme;
            LOG_INFO("Thème appliqué avec succès: " + themeName);
            return true;
        }
    }
    
    LOG_ERROR("Thème non trouvé: " + themeName);
    return false;
}

} // namespace openyolo
