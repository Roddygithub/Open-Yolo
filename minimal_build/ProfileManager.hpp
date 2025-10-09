#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <system_error>
#include <nlohmann/json.hpp>
#include "Logger.hpp"

namespace openyolo {

/**
 * @struct CursorProfile
 * @brief Représente un profil de configuration de curseur
 */
struct CursorProfile {
    std::string name;                // Nom du profil
    std::string theme;               // Thème de curseur
    int size = 24;                   // Taille du curseur
    bool useCustomTheme = false;     // Utiliser un thème personnalisé
    std::string customThemePath;     // Chemin vers le thème personnalisé
    std::map<std::string, std::string> cursorMapping;  // Mappage des curseurs personnalisés

    // Opérateur de conversion vers JSON
    nlohmann::json toJson() const;
    
    // Créer un profil à partir de JSON
    static CursorProfile fromJson(const nlohmann::json& j);
};

/**
 * @class ProfileManager
 * @brief Gère les profils de configuration des curseurs
 */
class ProfileManager {
public:
    /**
     * @brief Obtient l'instance unique du gestionnaire de profils
     */
    static ProfileManager& instance();
    
    /**
     * @brief Charge les profils disponibles
     * @return true si le chargement a réussi, false sinon
     */
    bool loadProfiles();
    
    /**
     * @brief Obtient la liste des profils disponibles
     */
    const std::vector<CursorProfile>& getProfiles() const { return m_profiles; }
    
    /**
     * @brief Obtient le profil actif
     */
    const CursorProfile* getActiveProfile() const { 
        return m_activeProfileIndex >= 0 && m_activeProfileIndex < static_cast<int>(m_profiles.size()) 
            ? &m_profiles[m_activeProfileIndex] 
            : nullptr; 
    }
    
    /**
     * @brief Définit le profil actif
     * @param index Index du profil à activer
     * @return true si le profil a été activé avec succès
     */
    bool setActiveProfile(int index);
    
    /**
     * @brief Crée un nouveau profil
     * @param name Nom du nouveau profil
     * @return true si la création a réussi
     */
    bool createProfile(const std::string& name);
    
    /**
     * @brief Supprime un profil
     * @param index Index du profil à supprimer
     * @return true si la suppression a réussi
     */
    std::error_code deleteProfile(int index);
    
    /**
     * @brief Sauvegarde le profil actif
     * @return true si la sauvegarde a réussi
     */
    bool saveCurrentProfile();

    /**
     * @brief Imports a profile from a file.
     * @param filePath Path to the profile file.
     * @return An error code on failure.
     */
    std::error_code importProfile(const std::string& filePath);

    /**
     * @brief Exports a profile to a file.
     * @param profileName The name of the profile to export.
     * @param filePath The path to export the profile to.
     * @return An error code on failure.
     */
    std::error_code exportProfile(const std::string& profileName, const std::string& filePath);
    
    // Désactive la copie et l'assignation
    ProfileManager(const ProfileManager&) = delete;
    ProfileManager& operator=(const ProfileManager&) = delete;

private:
    ProfileManager() = default;
    ~ProfileManager() = default;
    
    std::vector<CursorProfile> m_profiles;
    int m_activeProfileIndex = -1;
    std::filesystem::path m_profilesDir;
    
    // Charge un profil depuis un fichier
    std::error_code loadProfileFromFile(const std::filesystem::path& filePath);
    
    // Sauvegarde un profil dans un fichier
    std::error_code saveProfileToFile(const CursorProfile& profile) const;
    
    // Obtient le chemin du fichier de configuration
    std::filesystem::path getProfilePath(const std::string& profileName) const;
};

} // namespace openyolo
