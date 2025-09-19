#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <map>
#include <atomic>

namespace fs = std::filesystem;

namespace openyolo {

/**
 * @enum LogLevel
 * @brief Niveaux de sévérité des messages de log
 */
enum class LogLevel {
    TRACE,   // Niveau le plus bas, pour le débogage détaillé
    DEBUG,   // Informations de débogage
    INFO,    // Informations générales
    WARNING, // Avertissements
    ERROR,   // Erreurs qui n'empêchent pas le fonctionnement
    FATAL    // Erreurs critiques qui arrêtent l'application
};

/**
 * @class Logger
 * @brief Classe de gestion des logs de l'application
 * 
 * Cette classe fournit un système de journalisation thread-safe avec différents niveaux de sévérité,
 * rotation des fichiers de log et formatage personnalisable.
 */
class Logger {
public:
    /**
     * @brief Obtient l'instance unique du logger
     * @return Référence vers l'instance unique
     */
    static Logger& instance();

    // Suppression des constructeurs de copie et d'affectation
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Initialise le système de journalisation
     * @param logDir Répertoire où enregistrer les fichiers de log
     * @param maxFileSize Taille maximale d'un fichier de log en octets
     * @param maxFiles Nom maximum de fichiers de log à conserver
     * @param minLevel Niveau de log minimum à enregistrer
     * @param consoleOutput Si vrai, affiche les logs dans la console
     */
    void initialize(const fs::path& logDir = "logs", 
                   size_t maxFileSize = 5 * 1024 * 1024, // 5 Mo par défaut
                   size_t maxFiles = 10,
                   LogLevel minLevel = LogLevel::INFO,
                   bool consoleOutput = true);

    /**
     * @brief Définit le niveau de log minimum
     * @param level Niveau de log minimum
     */
    void setMinLevel(LogLevel level);

    /**
     * @brief Active ou désactive la sortie console
     * @param enable true pour activer, false pour désactiver
     */
    void enableConsoleOutput(bool enable);

    /**
     * @brief Enregistre un message de log
     * @param level Niveau de sévérité du message
     * @param message Message à enregistrer
     * @param file Fichier source (généralement __FILE__)
     * @param line Ligne source (généralement __LINE__)
     * @param function Fonction source (généralement __FUNCTION__)
     */
    void log(LogLevel level, 
             const std::string& message, 
             const std::string& file = "", 
             int line = 0, 
             const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau TRACE
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void trace(const std::string& message, 
               const std::string& file = "", 
               int line = 0, 
               const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau DEBUG
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void debug(const std::string& message, 
               const std::string& file = "", 
               int line = 0, 
               const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau INFO
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void info(const std::string& message, 
              const std::string& file = "", 
              int line = 0, 
              const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau WARNING
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void warning(const std::string& message, 
                 const std::string& file = "", 
                 int line = 0, 
                 const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau ERROR
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void error(const std::string& message, 
               const std::string& file = "", 
               int line = 0, 
               const std::string& function = "");

    /**
     * @brief Enregistre un message de niveau FATAL
     * @param message Message à enregistrer
     * @param file Fichier source
     * @param line Ligne source
     * @param function Fonction source
     */
    void fatal(const std::string& message, 
               const std::string& file = "", 
               int line = 0, 
               const std::string& function = "");

    /**
     * @brief Vide le tampon de sortie
     */
    void flush();

    /**
     * @brief Ferme le système de journalisation
     */
    void shutdown();

private:
    Logger();
    ~Logger();

    // Structure pour stocker les informations de contexte de log
    struct LogContext {
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;
        LogLevel level;
        std::string message;
        std::thread::id threadId;
    };

    // Méthodes privées
    void rotateLogs();
    void writeLog(const LogContext& context);
    std::string levelToString(LogLevel level) const;
    std::string getTimestampString(const std::chrono::system_clock::time_point& time) const;
    std::string formatLogMessage(const LogContext& context) const;
    std::string getLogFileName() const;

    // Membres
    std::ofstream m_logFile;
    std::string m_logFileName;
    fs::path m_logDir;
    size_t m_maxFileSize;
    size_t m_maxFiles;
    LogLevel m_minLevel;
    std::mutex m_mutex;
    bool m_consoleOutput;
    bool m_initialized;
    std::atomic<bool> m_shutdownRequested;
};

// Macro pour faciliter l'utilisation du logger
#define LOG_TRACE(msg) openyolo::Logger::instance().trace(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG(msg) openyolo::Logger::instance().debug(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO(msg) openyolo::Logger::instance().info(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg) openyolo::Logger::instance().warning(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg) openyolo::Logger::instance().error(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_FATAL(msg) openyolo::Logger::instance().fatal(msg, __FILE__, __LINE__, __FUNCTION__)

} // namespace openyolo

#endif // LOGGER_HPP
