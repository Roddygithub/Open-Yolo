// Standard C++
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <mutex>

// En-tête de la classe
#include "config/ConfigManager.hpp"

// POSIX
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace fs = std::filesystem;

namespace openyolo {

// Définition des membres statiques
std::mutex ConfigManager::m_mutex;
std::map<std::string, std::string> ConfigManager::m_values;

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::load(const fs::path& configPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Vérifier si le chemin est vide
    if (configPath.empty()) {
        throw std::invalid_argument("Le chemin de configuration ne peut pas être vide");
    }

    // Vérifier si le chemin est un répertoire
    if (fs::is_directory(configPath)) {
        throw std::invalid_argument("Le chemin de configuration est un répertoire: " + configPath.string());
    }
    
    // Vérifier si le fichier existe
    if (!fs::exists(configPath)) {
        // Créer le répertoire parent s'il n'existe pas
        try {
            const auto parentPath = configPath.parent_path();
            if (!parentPath.empty() && !fs::exists(parentPath)) {
                fs::create_directories(parentPath);
                // Définir les permissions appropriées (rwxr-xr-x)
                fs::permissions(parentPath, 
                              fs::perms::owner_all | 
                              fs::perms::group_read | fs::perms::group_exec |
                              fs::perms::others_read | fs::perms::others_exec,
                              fs::perm_options::replace);
            }
            // Créer un fichier de configuration par défaut
            save(configPath);
            return;
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Échec de la création du répertoire de configuration: " + 
                                  std::string(e.what()));
        }
    }

    // Vérifier les permissions du fichier
    try {
        const auto status = fs::status(configPath);
        if ((status.permissions() & fs::perms::owner_read) == fs::perms::none) {
            throw std::runtime_error("Permissions insuffisantes pour lire le fichier de configuration");
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Erreur lors de la vérification des permissions: " + 
                              std::string(e.what()));
    }

    // Ouvrir le fichier avec gestion d'erreur
    std::ifstream file(configPath, std::ios::in);
    if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier de configuration: " + 
                              configPath.string() + " (" + strerror(errno) + ")");
    }

    std::string currentSection;
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        
        // Ignorer les lignes vides et les commentaires
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Supprimer les espaces en début et fin de ligne
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Vérifier si c'est une section
        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Vérifier si c'est une paire clé=valeur
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Nettoyer la clé et la valeur
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Supprimer les guillemets autour de la valeur si présents
            if (value.length() >= 2 && 
                ((value.front() == '"' && value.back() == '"') || 
                 (value.front() == '\'' && value.back() == '\''))) {
                value = value.substr(1, value.length() - 2);
            }
            
            if (!key.empty() && !currentSection.empty()) {
                m_values[currentSection + "." + key] = value;
            }
        }
    }
}

void ConfigManager::save(const fs::path& configPath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Créer le répertoire parent s'il n'existe pas
    fs::create_directories(configPath.parent_path());
    
    std::ofstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Impossible d'écrire dans le fichier de configuration: " + configPath.string());
    }
    
    // Écrire l'en-tête
    file << "# Fichier de configuration Open-Yolo\n";
    file << "# Ce fichier est généré automatiquement, ne modifiez que si vous savez ce que vous faites\n\n";
    
    // Trier les clés par section pour un affichage plus propre
    std::map<std::string, std::map<std::string, std::string> > sections;
    
    for (const auto& pair : m_values) {
        size_t dotPos = pair.first.find('.');
        if (dotPos != std::string::npos) {
            std::string section = pair.first.substr(0, dotPos);
            std::string key = pair.first.substr(dotPos + 1);
            sections[section][key] = pair.second;
        }
    }
    
    // Écrire les sections et leurs clés
    for (const auto& sectionPair : sections) {
        file << "[" << sectionPair.first << "]\n";
        
        for (const auto& keyPair : sectionPair.second) {
            // Mettre entre guillemets si la valeur contient des espaces ou des caractères spéciaux
            bool needsQuotes = keyPair.second.find_first_of(" \t=;#\"'") != std::string::npos;
            
            if (needsQuotes) {
                file << "    " << keyPair.first << " = \"" << keyPair.second << "\"\n";
            } else {
                file << "    " << keyPair.first << " = " << keyPair.second << "\n";
            }
        }
        
        file << "\n";
    }
}

fs::path ConfigManager::getDefaultConfigPath() {
    const char* homeDir = getenv("XDG_CONFIG_HOME");
    if (!homeDir || homeDir[0] == '\0') {
        homeDir = getenv("HOME");
        if (!homeDir || homeDir[0] == '\0') {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                homeDir = pw->pw_dir;
            } else {
                // Dernier recours
                homeDir = ".";
            }
        }
        return fs::path(homeDir) / ".config" / "open-yolo" / "config.ini";
    }
    return fs::path(homeDir) / "open-yolo" / "config.ini";
}

void ConfigManager::loadDefaultConfig() {
    load(getDefaultConfigPath());
}

void ConfigManager::saveDefaultConfig() const {
    save(getDefaultConfigPath());
}

} // namespace openyolo
