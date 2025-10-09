#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

namespace openyolo {

/**
 * @enum LogLevel
 * @brief Niveaux de journalisation disponibles
 */
enum class LogLevel {
    TRACE,   ///< Niveau de trace très détaillé
    DEBUG,   ///< Informations de débogage
    INFO,    ///< Informations générales
    WARNING, ///< Avertissements
    ERROR,   ///< Erreurs
    FATAL    ///< Erreurs critiques
};

/**
 * @class Logger
 * @brief Gestionnaire de journalisation thread-safe
 */
class Logger {
public:
    /**
     * @brief Obtient l'instance unique du logger
     * @return Référence vers l'instance du logger
     */
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Initialise le logger
     * @param filename Chemin vers le fichier de log
     * @param minLevel Niveau de log minimum
     */
    void initialize(const std::string& filename = "openyolo.log", LogLevel minLevel = LogLevel::INFO);

    /**
     * @brief Écrit un message dans le journal
     * @param level Niveau de sévérité du message
     * @param message Message à enregistrer
     * @param file Fichier source (optionnel)
     * @param line Ligne du code source (optionnel)
     */
    void log(LogLevel level, const std::string& message, 
             const std::string& file = "", int line = 0);

    // Désactive la copie et l'assignation
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    ~Logger();

    std::ofstream m_logFile;
    LogLevel m_minLevel = LogLevel::INFO;
    std::mutex m_mutex;

    const char* levelToString(LogLevel level) const;
};

// Macros pratiques pour le logging
#define LOG_TRACE(msg)   openyolo::Logger::instance().log(openyolo::LogLevel::TRACE, msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg)   openyolo::Logger::instance().log(openyolo::LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg)    openyolo::Logger::instance().log(openyolo::LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) openyolo::Logger::instance().log(openyolo::LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg)   openyolo::Logger::instance().log(openyolo::LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_FATAL(msg)   openyolo::Logger::instance().log(openyolo::LogLevel::FATAL, msg, __FILE__, __LINE__)

} // namespace openyolo
