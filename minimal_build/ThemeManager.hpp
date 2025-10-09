#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace openyolo {

/**
 * @struct ThemeInfo
 * @brief Contient les informations sur un thème de curseur
 */
struct ThemeInfo {
    std::string name;           // Nom du thème
    std::string path;           // Chemin du dossier du thème
    std::string comment;        // Description du thème
    std::string screenshot;     // Chemin vers une capture d'écran (optionnel)
    bool isSystemTheme = false; // Si le thème est un thème système
};

/**
 * @class ThemeManager
 * @brief Gère les thèmes de curseurs
 */
class ThemeManager {
public:
    /**
     * @brief Obtient l'instance unique du gestionnaire de thèmes
     */
    static ThemeManager& instance();
    
    /**
     * @brief Charge les thèmes disponibles
     * @return true si le chargement a réussi, false sinon
     */
    bool loadThemes();
    
    /**
     * @brief Obtient la liste des thèmes disponibles
     */
    const std::vector<ThemeInfo>& getAvailableThemes() const { return m_availableThemes; }
    
    /**
     * @brief Applique un thème
     * @param themeName Nom du thème à appliquer
     * @return true si le thème a été appliqué avec succès
     */
    bool applyTheme(const std::string& themeName);
    
    /**
     * @brief Obtient le thème actuellement sélectionné
     */
    const ThemeInfo* getCurrentTheme() const { return m_currentTheme; }

    // Désactive la copie et l'assignation
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

private:
    ThemeManager() = default;
    
    std::vector<ThemeInfo> m_availableThemes;
    const ThemeInfo* m_currentTheme = nullptr;
    
    // Chemins standards pour les thèmes de curseurs
    const std::vector<std::string> m_themePaths = {
        "/usr/share/icons",
        "/usr/local/share/icons",
        "/home/" + std::string(getenv("USER")) + "/.local/share/icons",
        "/home/" + std::string(getenv("USER")) + "/.icons"
    };
    
    bool loadThemeFromDir(const std::string& dirPath, bool isSystemTheme);
    bool isCursorThemeDir(const std::string& dirPath) const;
};

} // namespace openyolo
