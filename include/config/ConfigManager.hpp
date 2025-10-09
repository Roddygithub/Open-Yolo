#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// Forward declaration pour l'amitié avec les validateurs
namespace openyolo::config::validators {
template <typename T>
class Validator;
}

// Déclaration anticipée pour l'opérateur de flux
namespace std {
std::istream& operator>>(std::istream& is, std::filesystem::path& path);
std::ostream& operator<<(std::ostream& os, const std::filesystem::path& path);
}

namespace fs = std::filesystem;

namespace openyolo {

/**
 * @namespace config
 * @brief Espace de noms pour les fonctionnalités liées à la configuration
 */
namespace config {

/**
 * @class ConfigError
 * @brief Exception levée en cas d'erreur de configuration
 */
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& what) : std::runtime_error(what) {}
};

/**
 * @class ValidationError
 * @brief Exception levée en cas d'échec de validation
 */
class ValidationError : public ConfigError {
public:
    explicit ValidationError(const std::string& what) : ConfigError("Validation error: " + what) {}
};

/**
 * @class IOError
 * @brief Exception levée en cas d'erreur d'E/S
 */
class IOError : public ConfigError {
public:
    explicit IOError(const std::string& what) : ConfigError("I/O error: " + what) {}
};

/**
 * @class ConfigBackup
 * @brief Gère les sauvegardes de fichiers de configuration
 */
class ConfigBackup {
public:
    /**
     * @brief Crée une sauvegarde du fichier de configuration
     * @param configPath Chemin vers le fichier de configuration
     * @return Chemin vers la sauvegarde ou chemin vide si la sauvegarde a échoué
     */
    static std::filesystem::path createBackup(const std::filesystem::path& configPath);

    /**
     * @brief Restaure la configuration depuis une sauvegarde
     * @param configPath Chemin vers le fichier de configuration
     * @param backupPath Chemin vers la sauvegarde
     * @return true si la restauration a réussi, false sinon
     */
    static bool restoreFromBackup(const std::filesystem::path& configPath,
                                  const std::filesystem::path& backupPath);

private:
    static std::string generateBackupSuffix();
};

// Déclaration anticipée pour l'amitié avec les validateurs
template <typename T>
class ConfigValue;

/**
 * @class ConfigManager
 * @brief Gestionnaire de configuration pour l'application Open-Yolo
 *
 * Cette classe fournit une interface thread-safe pour gérer la configuration
 * de l'application. Elle implémente le modèle Singleton pour garantir une
 * seule instance de configuration dans toute l'application.
 *
 * Exemple d'utilisation :
 * @code
 * // Obtenir l'instance du gestionnaire
 * auto& config = ConfigManager::instance();
 *
 * // Charger la configuration
 * try {
 *     config.load("config.ini");
 * } catch (const ConfigError& e) {
 *     std::cerr << "Erreur de configuration: " << e.what() << std::endl;
 *     return 1;
 * }
 *
 * // Lire une valeur
 * int port = config.getValue("server", "port", 8080);
 *
 * // Écrire une valeur
 * config.setValue("server", "port", 9000);
 *
 * // Sauvegarder la configuration
 * try {
 *     config.save("config.ini");
 * } catch (const IOError& e) {
 *     std::cerr << "Erreur lors de la sauvegarde: " << e.what() << std::endl;
 * }
 * @endcode
 */
class ConfigManager {
    friend class validators::Validator<ConfigManager>;

public:
    /**
     * @brief Obtient l'instance unique du gestionnaire de configuration
     * @return Référence vers l'instance unique
     */
    static ConfigManager& instance();

    // Suppression des constructeurs de copie et d'affectation
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    /**
     * @brief Charge la configuration depuis un fichier
     * @param configPath Chemin vers le fichier de configuration
     * @param createIfNotExists Créer le fichier s'il n'existe pas
     * @throws ConfigError Si le chargement échoue
     * @throws IOError Si une erreur d'E/S survient
     *
     * Le fichier de configuration doit être au format INI avec la structure suivante :
     * @code
     * [section1]
     * key1=value1
     * key2=value2
     *
     * [section2]
     * key3=value3
     * @endcode
     */
    void load(const std::filesystem::path& configPath, bool createIfNotExists = false);

    /**
     * @brief Enregistre la configuration dans un fichier
     * @param configPath Chemin vers le fichier de configuration
     * @param createBackup Créer une sauvegarde avant d'écraser le fichier
     * @throws IOError Si l'enregistrement échoue
     * @throws ConfigError Si une erreur de configuration survient
     *
     * Si createBackup est vrai, une copie de sauvegarde du fichier existant
     * sera créée avec l'extension .bak suivi d'un timestamp.
     */
    void save(const std::filesystem::path& configPath, bool createBackup = true) const;

    /**
     * @brief Définit une valeur de configuration
     * @tparam T Type de la valeur (doit être sérialisable via std::ostream)
     * @param section Section de configuration (ne doit pas être vide)
     * @param key Clé de configuration (ne doit pas être vide)
     * @param value Valeur à définir
     * @param validate Si vrai, valide la valeur avant de la définir
     * @throws ValidationError Si la validation échoue
     * @throws std::invalid_argument Si section ou key est vide
     *
     * Exemple :
     * @code
     * config.setValue("network", "port", 8080);
     * config.setValue("display", "fullscreen", true);
     * config.setValue("user", "name", "John Doe");
     * @endcode
     */
    template <typename T>
    void setValue(const std::string& section, const std::string& key, const T& value,
                  bool validate = true) {
        if (section.empty() || key.empty()) {
            throw std::invalid_argument("Section and key cannot be empty");
        }

        std::string fullKey = section + "." + key;
        std::ostringstream ss;
        ss << value;

        if (validate) {
            validateValue(section, key, ss.str());
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_values[fullKey] = ss.str();
    }

    /**
     * @brief Obtient une valeur de configuration
     * @tparam T Type de la valeur à récupérer
     * @param section Section de configuration
     * @param key Clé de configuration
     * @param defaultValue Valeur par défaut à retourner si la clé n'existe pas
     * @return La valeur de configuration ou la valeur par défaut
     * @throws ValidationError Si la valeur existe mais ne peut pas être convertie au type demandé
     *
     * Exemple :
     * @code
     * int port = config.getValue("network", "port", 8080);
     * bool fullscreen = config.getValue("display", "fullscreen", false);
     * std::string username = config.getValue("user", "name", "guest");
     * @endcode
     */
    template <typename T>
    T getValue(const std::string& section, const std::string& key,
               const T& defaultValue = T()) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string fullKey = section + "." + key;
        auto it = m_values.find(fullKey);

        if (it == m_values.end()) {
            return defaultValue;
        }

        try {
            std::istringstream ss(it->second);
            T value;

            if constexpr (std::is_same_v<T, std::string>) {
                // Pour les chaînes, on prend tout le contenu
                value = it->second;
            } else if constexpr (std::is_same_v<T, bool>) {
                // Gestion spéciale pour les booléens
                std::string str = it->second;
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                if (str == "true" || str == "1" || str == "yes" || str == "on") {
                    value = true;
                } else if (str == "false" || str == "0" || str == "no" || str == "off") {
                    value = false;
                } else {
                    throw ValidationError("Valeur booléenne invalide: " + it->second);
                }
            } else {
                // Pour les autres types, on utilise l'opérateur de flux
                if (!(ss >> value)) {
                    throw ValidationError("Impossible de convertir la valeur '" + it->second +
                                          "' vers le type demandé");
                }
            }

            return value;
        } catch (const std::exception& e) {
            throw ValidationError(std::string("Erreur de conversion pour ") + fullKey + ": " +
                                  e.what());
        }
    }

    /**
     * @brief Spécialisation pour les tableaux de caractères (chaînes C)
     */
    std::string getValue(const std::string& section, const std::string& key,
                         const char* defaultValue) const {
        return getValue<std::string>(section, key, std::string(defaultValue));
    }

    /**
     * @brief Spécialisation pour les chemins de fichiers
     */
    std::filesystem::path getValue(const std::string& section, const std::string& key,
                                   const std::filesystem::path& defaultValue) const {
        return getValue<std::filesystem::path>(section, key, defaultValue);
    }

    /**
     * @brief Vérifie si une clé de configuration existe
     * @param section Section de configuration
     * @param key Clé de configuration
     * @return true si la clé existe, false sinon
     */
    bool hasValue(const std::string& section, const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_values.find(section + "." + key) != m_values.end();
    }

    /**
     * @brief Supprime une clé de configuration
     * @param section Section de configuration
     * @param key Clé de configuration
     * @return true si la clé existait et a été supprimée, false sinon
     */
    bool removeValue(const std::string& section, const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_values.erase(section + "." + key) > 0;
    }

    /**
     * @brief Efface toutes les valeurs de configuration
     * @warning Cette opération est irréversible
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_values.clear();
    }

    /**
     * @brief Charge la configuration depuis une chaîne au format INI
     * @param iniData Données au format INI
     * @throws ConfigError Si le format est invalide
     */
    void loadFromString(const std::string& iniData);

    /**
     * @brief Sauvegarde la configuration dans une chaîne au format INI
     * @return La configuration au format INI
     */
    std::string saveToString() const;

    /**
     * @brief Valide une valeur avant de la définir
     * @param section Section de configuration
     * @param key Clé de configuration
     * @param value Valeur à valider
     * @throws ValidationError Si la validation échoue
     */
    void validateValue(const std::string& section, const std::string& key,
                       const std::string& value) const {
        // Implémentation par défaut : toujours valide
        // Peut être surchargée par des classes dérivées pour ajouter des validations
        (void)section;
        (void)key;
        (void)value;
    }

    /**
     * @brief Obtient toutes les paires clé-valeur pour une section
     * @param section Section de configuration
     * @return Un vecteur de paires (clé, valeur)
     */
    std::vector<std::pair<std::string, std::string>> getSection(const std::string& section) const {
        std::vector<std::pair<std::string, std::string>> result;
        std::string prefix = section + ".";
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const auto& [key, value] : m_values) {
            if (key.compare(0, prefix.length(), prefix) == 0) {
                result.emplace_back(key.substr(prefix.length()), value);
            }
        }

        return result;
    }
    if (it == m_values.end()) {
        return defaultValue;
    }

    const std::string& value = it->second;
    return (value == "true" || value == "1" || value == "yes" || value == "on");
}

    /**
     * @brief Vérifie si une clé de configuration existe
     * @param section Section de configuration
     * @param key Clé de configuration
     */
    bool hasKey(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_values.find(section + "." + key) != m_values.end();
}

/**
 * @brief Supprime une clé de configuration
 * @param section Section de configuration
 * @param key Clé de configuration à supprimer
 * @return true si la clé a été supprimée, false si elle n'existait pas
 */
bool removeKey(const std::string& section, const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_values.erase(section + "." + key) > 0;
}

/**
 * @brief Efface toutes les clés d'une section
 * @param section Section à effacer
 * @return Nombre de clés supprimées
 */
size_t clearSection(const std::string& section) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string prefix = section + ".";
    size_t count = 0;

    for (auto it = m_values.begin(); it != m_values.end();) {
        if (it->first.compare(0, prefix.length(), prefix) == 0) {
            it = m_values.erase(it);
            ++count;
        } else {
            ++it;
        }
    }

    return count;
}

/**
 * @brief Obtient le chemin du fichier de configuration par défaut
 * @return Chemin vers le fichier de configuration
 */
static fs::path getDefaultConfigPath();

/**
 * @brief Charge la configuration à partir du fichier par défaut
 */
void loadDefaultConfig();

/**
 * @brief Enregistre la configuration dans le fichier par défaut
 */
void saveDefaultConfig() const;

private:
/**
 * @brief Constructeur privé pour forcer l'utilisation du Singleton
 */
ConfigManager() = default;
// Spécialisation pour std::filesystem::path
namespace std {
inline std::istream& operator>>(std::istream& is, std::filesystem::path& path) {
    std::string str;
    is >> str;
    path = str;
    return is;
}

inline std::ostream& operator<<(std::ostream& os, const std::filesystem::path& path) {
    return os << path.string();
}
}

#endif  // CONFIG_MANAGER_HPP
