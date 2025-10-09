#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <mutex>

namespace fs = std::filesystem;

namespace openyolo {

/**
 * @class ConfigManager
 * @brief Gestionnaire de configuration pour l'application Open-Yolo
 * 
 * Cette classe gère le chargement, la sauvegarde et l'accès aux paramètres
 * de configuration de l'application. Elle utilise un fichier INI pour stocker
 * les paramètres de manière persistante.
 */
class ConfigManager {
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
     * @brief Charge la configuration depuis le fichier
     * @param configPath Chemin vers le fichier de configuration
     * @throws std::runtime_error Si le chargement échoue
     */
    void load(const fs::path& configPath);

    /**
     * @brief Enregistre la configuration dans le fichier
     * @param configPath Chemin vers le fichier de configuration
     * @throws std::runtime_error Si l'enregistrement échoue
     */
    void save(const fs::path& configPath) const;

    /**
     * @brief Définit une valeur de configuration
     * @tparam T Type de la valeur
     * @param section Section de configuration
     * @param key Clé de configuration
     * @param value Valeur à définir
     */
    template<typename T>
    void setValue(const std::string& section, const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string fullKey = section + "." + key;
        std::ostringstream ss;
        ss << value;
        m_values[fullKey] = ss.str();
    }

    /**
     * @brief Obtient une valeur de configuration
     * @param section Section de configuration
     * @param key Clé de configuration
     * @param defaultValue Valeur par défaut si la clé n'existe pas
     * @return La valeur de configuration ou la valeur par défaut
     */
    // Méthode générique pour obtenir une valeur de configuration
    template<typename T>
    T getValue(const std::string& section, const std::string& key, const T& defaultValue = T()) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string fullKey = section + "." + key;
        auto it = m_values.find(fullKey);
        if (it == m_values.end()) {
            return defaultValue;
        }

        std::istringstream ss(it->second);
        T value;
        ss >> value;
        return value;
    }
    
    // Spécialisation pour std::string
    std::string getValue(const std::string& section, const std::string& key, const char* defaultValue) const {
        return getValue<std::string>(section, key, std::string(defaultValue));
    }
    
    // Spécialisation pour bool
    bool getValue(const std::string& section, const std::string& key, bool defaultValue) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string fullKey = section + "." + key;
        auto it = m_values.find(fullKey);
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
        
        for (auto it = m_values.begin(); it != m_values.end(); ) {
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
    ConfigManager() = default;
    ~ConfigManager() = default;

    // Membres statiques
    static std::mutex m_mutex;
    static std::map<std::string, std::string> m_values;
};

} // namespace openyolo

#endif // CONFIG_MANAGER_HPP
