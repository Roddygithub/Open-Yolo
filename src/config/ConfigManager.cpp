// Standard C++
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

// En-tête de la classe
#include "config/ConfigManager.hpp"

// POSIX
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Pour la journalisation (à adapter selon votre système de logs)
#ifndef LOG_ERROR
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define LOG_WARN(msg) std::cerr << "[WARN] " << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#endif

namespace fs = std::filesystem;

namespace openyolo {
namespace config {

// Définition des membres statiques
std::mutex ConfigManager::m_mutex;
std::unordered_map<std::string, std::string> ConfigManager::m_values;

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

// Implémentation de ConfigBackup
std::string ConfigBackup::generateBackupSuffix() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

std::filesystem::path ConfigBackup::createBackup(const std::filesystem::path& configPath) {
    if (!fs::exists(configPath)) {
        LOG_WARN("Impossible de créer une sauvegarde: le fichier n'existe pas: " +
                 configPath.string());
        return {};
    }

    try {
        std::string backupName = configPath.filename().string() + ".bak_" + generateBackupSuffix();
        fs::path backupPath = configPath.parent_path() / backupName;

        // Copier le fichier
        fs::copy_file(configPath, backupPath, fs::copy_options::overwrite_existing);
        LOG_INFO("Sauvegarde créée: " + backupPath.string());
        return backupPath;
    } catch (const std::exception& e) {
        LOG_ERROR("Échec de la création de la sauvegarde: " + std::string(e.what()));
        return {};
    }
}

bool ConfigBackup::restoreFromBackup(const std::filesystem::path& configPath,
                                     const std::filesystem::path& backupPath) {
    if (!fs::exists(backupPath)) {
        LOG_ERROR("Le fichier de sauvegarde n'existe pas: " + backupPath.string());
        return false;
    }

    try {
        fs::copy_file(backupPath, configPath, fs::copy_options::overwrite_existing);
        LOG_INFO("Configuration restaurée depuis la sauvegarde: " + backupPath.string());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Échec de la restauration depuis la sauvegarde: " + std::string(e.what()));
        return false;
    }
}

// Implémentation de ConfigManager
void ConfigManager::load(const fs::path& configPath, bool createIfNotExists) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Vérifier si le chemin est vide
    if (configPath.empty()) {
        throw ConfigError("Le chemin de configuration ne peut pas être vide");
    }

    // Vérifier si le chemin est un répertoire
    if (fs::is_directory(configPath)) {
        throw ConfigError("Le chemin de configuration est un répertoire: " + configPath.string());
    }

    // Vérifier si le fichier existe
    if (!fs::exists(configPath)) {
        if (createIfNotExists) {
            try {
                // Créer le répertoire parent s'il n'existe pas
                const auto parentPath = configPath.parent_path();
                if (!parentPath.empty() && !fs::exists(parentPath)) {
                    fs::create_directories(parentPath);
                    // Définir les permissions appropriées (rwxr-xr-x)
                    fs::permissions(parentPath,
                                    fs::perms::owner_all | fs::perms::group_read |
                                        fs::perms::group_exec | fs::perms::others_read |
                                        fs::perms::others_exec,
                                    fs::perm_options::replace);
                }
                // Créer un fichier de configuration vide
                std::ofstream file(configPath);
                if (!file) {
                    throw IOError("Impossible de créer le fichier de configuration: " +
                                  configPath.string());
                }
                LOG_INFO("Fichier de configuration créé: " + configPath.string());
                return;
            } catch (const fs::filesystem_error& e) {
                throw IOError("Échec de la création du répertoire de configuration: " +
                              std::string(e.what()));
            }
        } else {
            throw IOError("Le fichier de configuration n'existe pas: " + configPath.string());
        }
    }

    // Vérifier les permissions du fichier
    try {
        const auto status = fs::status(configPath);
        if ((status.permissions() & fs::perms::owner_read) == fs::perms::none) {
            throw IOError("Permissions insuffisantes pour lire le fichier de configuration");
        }
    } catch (const fs::filesystem_error& e) {
        throw IOError("Erreur lors de la vérification des permissions: " + std::string(e.what()));
    }

    // Charger le contenu du fichier dans une chaîne
    std::string content;
    try {
        std::ifstream file(configPath, std::ios::in | std::ios::binary);
        if (!file) {
            throw IOError("Impossible d'ouvrir le fichier de configuration: " +
                          configPath.string() + " (" + strerror(errno) + ")");
        }

        // Lire tout le contenu du fichier
        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        content = contentStream.str();

        if (file.bad()) {
            throw IOError("Erreur lors de la lecture du fichier de configuration");
        }
    } catch (const std::exception& e) {
        throw IOError(std::string("Erreur lors de la lecture du fichier: ") + e.what());
    }

    // Parser le contenu
    try {
        loadFromString(content);
        LOG_INFO("Configuration chargée depuis: " + configPath.string());
    } catch (const ConfigError&) {
        throw;  // Relancer les erreurs de configuration telles quelles
    } catch (const std::exception& e) {
        throw ConfigError(std::string("Erreur lors du parsing de la configuration: ") + e.what());
    }
}

void ConfigManager::loadFromString(const std::string& iniData) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::istringstream stream(iniData);
    std::string line;
    std::string currentSection;
    size_t lineNumber = 0;

    // Vider les valeurs existantes
    m_values.clear();

    while (std::getline(stream, line)) {
        ++lineNumber;

        // Supprimer les espaces en début et fin de ligne
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Ignorer les lignes vides et les commentaires
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Vérifier si c'est une section
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Trouver le séparateur '='
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            LOG_WARN("Ligne " + std::to_string(lineNumber) + ": format invalide (manque '=')");
            continue;
        }

        // Extraire la clé et la valeur
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        // Nettoyer les espaces autour de la clé et de la valeur
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // Supprimer les guillemets autour de la valeur si présents
        if (value.length() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                    (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.length() - 2);
        }

        // Ajouter la paire clé-valeur à la section courante
        if (!currentSection.empty() && !key.empty()) {
            m_values[currentSection + "." + key] = value;
        }
    }
}

std::string ConfigManager::saveToString() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream result;

    // Grouper les clés par section
    std::map<std::string, std::map<std::string, std::string>> sections;

    for (const auto& [fullKey, value] : m_values) {
        size_t dotPos = fullKey.find('.');
        if (dotPos == std::string::npos) {
            continue;  // Ignorer les clés sans section
        }

        std::string section = fullKey.substr(0, dotPos);
        std::string key = fullKey.substr(dotPos + 1);
        sections[section][key] = value;
    }

    // Écrire les sections et leurs clés
    bool firstSection = true;
    for (const auto& [section, keys] : sections) {
        if (!firstSection) {
            result << "\n";
        }
        firstSection = false;

        // Écrire l'en-tête de section
        result << "[" << section << "]\n";

        // Écrire les paires clé-valeur
        for (const auto& [key, value] : keys) {
            // Vérifier si la valeur nécessite des guillemets
            bool needsQuotes =
                value.empty() || value.find_first_of(" \t=;#\"'") != std::string::npos;

            result << key << "=";
            if (needsQuotes) {
                result << "\"" << value << "\"";
            } else {
                result << value;
            }
            result << "\n";
        }
    }

    return result.str();
}

void ConfigManager::save(const fs::path& configPath, bool createBackup) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (configPath.empty()) {
        throw std::invalid_argument("Le chemin de configuration ne peut pas être vide");
    }

    // Créer une sauvegarde si demandé et si le fichier existe déjà
    if (createBackup && fs::exists(configPath)) {
        try {
            auto backupPath = ConfigBackup::createBackup(configPath);
            if (!backupPath.empty()) {
                LOG_INFO("Sauvegarde créée: " + backupPath.string());
            }
        } catch (const std::exception& e) {
            LOG_WARN("Échec de la création de la sauvegarde: " + std::string(e.what()));
            // Continuer même si la sauvegarde échoue
        }
    }

    // Créer le répertoire parent s'il n'existe pas
    const auto parentPath = configPath.parent_path();
    if (!parentPath.empty() && !fs::exists(parentPath)) {
        try {
            fs::create_directories(parentPath);
            // Définir les permissions appropriées (rwxr-xr-x)
            fs::permissions(parentPath,
                            fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec |
                                fs::perms::others_read | fs::perms::others_exec,
                            fs::perm_options::replace);
        } catch (const fs::filesystem_error& e) {
            throw IOError("Échec de la création du répertoire: " + parentPath.string() + ": " +
                          e.what());
        }
    }

    // Écrire le contenu dans un fichier temporaire d'abord
    fs::path tempPath = configPath;
    tempPath += ".tmp";

    try {
        // Écrire dans un fichier temporaire
        std::ofstream file(tempPath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file) {
            throw IOError("Impossible d'écrire dans le fichier temporaire: " + tempPath.string() +
                          " (" + strerror(errno) + ")");
        }

        // Générer le contenu INI
        std::string content = saveToString();
        file << content;
        file.close();

        if (file.bad()) {
            throw IOError("Erreur lors de l'écriture dans le fichier temporaire");
        }

        // Remplacer le fichier existant par le fichier temporaire
        if (fs::exists(configPath)) {
            fs::remove(configPath);
        }
        fs::rename(tempPath, configPath);

        // Définir les permissions appropriées (rw-r--r--)
        fs::permissions(configPath,
                        fs::perms::owner_read | fs::perms::owner_write | fs::perms::group_read |
                            fs::perms::others_read,
                        fs::perm_options::replace);

        LOG_INFO("Configuration sauvegardée dans: " + configPath.string());

    } catch (const std::exception& e) {
        // Supprimer le fichier temporaire en cas d'erreur
        try {
            if (fs::exists(tempPath))
                fs::remove(tempPath);
        } catch (...) {
        }
        throw IOError(std::string("Erreur lors de la sauvegarde de la configuration: ") + e.what());
    }
}

// Implémentation des méthodes statiques pour la compatibilité
fs::path ConfigManager::getDefaultConfigPath() {
    static const std::string CONFIG_DIR = ".config";
    static const std::string APP_NAME = "open-yolo";
    static const std::string CONFIG_FILE = "config.ini";

    // Essayer XDG_CONFIG_HOME d'abord
    const char* configHome = std::getenv("XDG_CONFIG_HOME");
    if (configHome && *configHome) {
        return fs::path(configHome) / APP_NAME / CONFIG_FILE;
    }

    // Sinon, utiliser $HOME/.config
    const char* homeDir = std::getenv("HOME");
    if (!homeDir || !*homeDir) {
        struct passwd* pw = getpwuid(getuid());
        if (pw && pw->pw_dir) {
            homeDir = pw->pw_dir;
        } else {
            // Dernier recours : utiliser le répertoire courant
            return CONFIG_FILE;
        }
    }

    return fs::path(homeDir) / CONFIG_DIR / APP_NAME / CONFIG_FILE;
}

void ConfigManager::loadDefaultConfig() {
    try {
        load(getDefaultConfigPath(), true);
    } catch (const IOError& e) {
        LOG_ERROR("Impossible de charger la configuration par défaut: " + std::string(e.what()));
        throw;
    }
}

void ConfigManager::saveDefaultConfig() const {
    try {
        save(getDefaultConfigPath(), true);
    } catch (const IOError& e) {
        LOG_ERROR("Impossible de sauvegarder la configuration par défaut: " +
                  std::string(e.what()));
        throw;
    }
}

}  // namespace config
}  // namespace openyolo
