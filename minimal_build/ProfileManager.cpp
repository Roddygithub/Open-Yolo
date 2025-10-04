#include "ProfileManager.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

namespace openyolo {

using json = nlohmann::json;
namespace fs = std::filesystem;

// Implémentation des méthodes de CursorProfile
json CursorProfile::toJson() const {
    json j;
    j["name"] = name;
    j["theme"] = theme;
    j["size"] = size;
    j["useCustomTheme"] = useCustomTheme;
    if (useCustomTheme) {
        j["customThemePath"] = customThemePath;
    }
    
    if (!cursorMapping.empty()) {
        json mappingJson;
        for (const auto& [key, value] : cursorMapping) {
            mappingJson[key] = value;
        }
        j["cursorMapping"] = mappingJson;
    }
    
    return j;
}

CursorProfile CursorProfile::fromJson(const json& j) {
    CursorProfile profile;
    profile.name = j.value("name", "Nouveau profil");
    profile.theme = j.value("theme", "default");
    profile.size = j.value("size", 24);
    profile.useCustomTheme = j.value("useCustomTheme", false);
    
    if (profile.useCustomTheme) {
        profile.customThemePath = j.value("customThemePath", "");
    }
    
    if (j.contains("cursorMapping") && j["cursorMapping"].is_object()) {
        for (const auto& [key, value] : j["cursorMapping"].items()) {
            if (value.is_string()) {
                profile.cursorMapping[key] = value.get<std::string>();
            }
        }
    }
    
    return profile;
}

// Implémentation de ProfileManager
ProfileManager& ProfileManager::instance() {
    static ProfileManager instance;
    return instance;
}

bool ProfileManager::loadProfiles() {
    m_profiles.clear();
    
    // Créer le répertoire des profils s'il n'existe pas
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            homeDir = pw->pw_dir;
        } else {
            LOG_ERROR("Impossible de déterminer le répertoire personnel");
            return false;
        }
    }
    
    m_profilesDir = fs::path(homeDir) / ".config" / "open-yolo" / "profiles";
    
    try {
        if (!fs::exists(m_profilesDir)) {
            if (!fs::create_directories(m_profilesDir)) {
                LOG_ERROR("Échec de la création du répertoire des profils: " + m_profilesDir.string());
                return false;
            }
            LOG_INFO("Répertoire des profils créé: " + m_profilesDir.string());
            
            // Créer un profil par défaut
            CursorProfile defaultProfile;
            defaultProfile.name = "Par défaut";
            defaultProfile.theme = "Adwaita";
            m_profiles.push_back(std::move(defaultProfile));
            saveProfileToFile(m_profiles.back());
            return true;
        }
        
        // Charger les profils existants
        for (const auto& entry : fs::directory_iterator(m_profilesDir)) {
            if (entry.path().extension() == ".json") {
                if (!loadProfileFromFile(entry.path())) {
                    LOG_WARNING("Échec du chargement du profil: " + entry.path().string());
                }
            }
        }
        
        if (m_profiles.empty()) {
            LOG_WARNING("Aucun profil trouvé, création d'un profil par défaut");
            CursorProfile defaultProfile;
            defaultProfile.name = "Par défaut";
            defaultProfile.theme = "Adwaita";
            m_profiles.push_back(std::move(defaultProfile));
            saveProfileToFile(m_profiles.back());
        }
        
        // Activer le premier profil par défaut
        m_activeProfileIndex = 0;
        
        LOG_INFO(std::to_string(m_profiles.size()) + " profils chargés");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du chargement des profils: " + std::string(e.what()));
        return false;
    }
}

bool ProfileManager::loadProfileFromFile(const fs::path& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Impossible d'ouvrir le fichier de profil: " + filePath.string());
            return false;
        }
        
        json j;
        file >> j;
        
        CursorProfile profile = CursorProfile::fromJson(j);
        m_profiles.push_back(std::move(profile));
        
        LOG_DEBUG("Profil chargé: " + filePath.string());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du chargement du profil " + filePath.string() + ": " + e.what());
        return false;
    }
}

bool ProfileManager::saveProfileToFile(const CursorProfile& profile) const {
    try {
        fs::path filePath = getProfilePath(profile.name);
        std::ofstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Impossible de créer le fichier de profil: " + filePath.string());
            return false;
        }
        
        json j = profile.toJson();
        file << j.dump(4); // Indentation de 4 espaces pour une meilleure lisibilité
        
        LOG_DEBUG("Profil sauvegardé: " + filePath.string());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la sauvegarde du profil: " + std::string(e.what()));
        return false;
    }
}

bool ProfileManager::setActiveProfile(int index) {
    if (index < 0 || index >= static_cast<int>(m_profiles.size())) {
        LOG_ERROR("Index de profil invalide: " + std::to_string(index));
        return false;
    }
    
    m_activeProfileIndex = index;
    const auto& profile = m_profiles[index];
    
    LOG_INFO("Profil actif: " + profile.name);
    
    // Ici, nous devrions appliquer les paramètres du profil
    // Par exemple, changer le thème du curseur, la taille, etc.
    
    return saveCurrentProfile();
}

bool ProfileManager::createProfile(const std::string& name) {
    if (name.empty()) {
        LOG_ERROR("Le nom du profil ne peut pas être vide");
        return false;
    }
    
    // Vérifier si un profil avec ce nom existe déjà
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&name](const CursorProfile& p) { return p.name == name; });
    
    if (it != m_profiles.end()) {
        LOG_ERROR("Un profil avec ce nom existe déjà: " + name);
        return false;
    }
    
    // Créer un nouveau profil basé sur le profil actuel ou avec des valeurs par défaut
    CursorProfile newProfile;
    if (m_activeProfileIndex >= 0 && m_activeProfileIndex < static_cast<int>(m_profiles.size())) {
        newProfile = m_profiles[m_activeProfileIndex];
    }
    newProfile.name = name;
    
    // Sauvegarder le nouveau profil
    if (!saveProfileToFile(newProfile)) {
        return false;
    }
    
    m_profiles.push_back(std::move(newProfile));
    m_activeProfileIndex = m_profiles.size() - 1;
    
    LOG_INFO("Nouveau profil créé: " + name);
    return true;
}

bool ProfileManager::deleteProfile(int index) {
    if (index < 0 || index >= static_cast<int>(m_profiles.size())) {
        LOG_ERROR("Index de profil invalide: " + std::to_string(index));
        return false;
    }
    
    if (m_profiles.size() <= 1) {
        LOG_ERROR("Impossible de supprimer le dernier profil");
        return false;
    }
    
    const std::string profileName = m_profiles[index].name;
    fs::path filePath = getProfilePath(profileName);
    
    // Supprimer le fichier de profil
    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Échec de la suppression du fichier de profil " + filePath.string() + ": " + e.what());
        return false;
    }
    
    // Supprimer le profil de la liste
    m_profiles.erase(m_profiles.begin() + index);
    
    // Ajuster l'index du profil actif si nécessaire
    if (m_activeProfileIndex >= index) {
        m_activeProfileIndex = std::max(0, m_activeProfileIndex - 1);
    }
    
    LOG_INFO("Profil supprimé: " + profileName);
    return true;
}

bool ProfileManager::saveCurrentProfile() {
    if (m_activeProfileIndex < 0 || m_activeProfileIndex >= static_cast<int>(m_profiles.size())) {
        LOG_ERROR("Aucun profil actif à sauvegarder");
        return false;
    }
    
    return saveProfileToFile(m_profiles[m_activeProfileIndex]);
}

fs::path ProfileManager::getProfilePath(const std::string& profileName) const {
    // Créer un nom de fichier valide à partir du nom du profil
    std::string fileName = profileName;
    std::replace_if(fileName.begin(), fileName.end(),
        [](char c) { return !std::isalnum(c) && c != '-' && c != '_'; }, '_');
    
    return m_profilesDir / (fileName + ".json");
}

} // namespace openyolo
