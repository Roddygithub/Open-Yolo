#include "../../include/log/Logger.hpp"

// Standard C++
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// La structure LogContext est déjà définie dans Logger.hpp, pas besoin de la redéfinir ici

// Définir l'alias pour std::filesystem
namespace fs = std::filesystem;

// Linux/Unix specific
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace openyolo {

// Implémentation du singleton
Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

// Constructeur privé
openyolo::Logger::Logger()
    : m_maxFileSize(5 * 1024 * 1024)  // 5 Mo par défaut
      ,
      m_maxFiles(10),
      m_minLevel(LogLevel::INFO),
      m_consoleOutput(true),
      m_initialized(false),
      m_shutdownRequested(false),
      m_logFileName("open-yolo.log"),
      m_logThread() {
    // Le répertoire de logs par défaut est ~/.local/share/open-yolo/logs
    const char* homeDir = getenv("XDG_DATA_HOME");
    if (!homeDir || homeDir[0] == '\0') {
        homeDir = getenv("HOME");
        if (!homeDir || homeDir[0] == '\0') {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                homeDir = pw->pw_dir;
            } else {
                homeDir = ".";
            }
        }
        m_logDir = fs::path(homeDir) / ".local" / "share" / "open-yolo" / "logs";
    } else {
        m_logDir = fs::path(homeDir) / "open-yolo" / "logs";
    }
}

// Destructeur
openyolo::Logger::~Logger() { shutdown(); }

void openyolo::Logger::initialize(const fs::path& logDir, size_t maxFileSize, size_t maxFiles,
                                  LogLevel minLevel, bool consoleOutput) {
    if (m_initialized) {
        return;  // Déjà initialisé
    }

    // Validation des paramètres
    if (maxFileSize == 0) {
        throw std::invalid_argument("maxFileSize must be greater than 0");
    }
    if (maxFiles == 0) {
        throw std::invalid_argument("maxFiles must be greater than 0");
    }

    try {
        // Stocker les paramètres
        m_maxFileSize = maxFileSize;
        m_maxFiles = maxFiles;
        m_minLevel = minLevel;
        m_consoleOutput = consoleOutput;

        // Créer le répertoire de logs s'il n'existe pas
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
            // Définir les permissions du répertoire (rwxr-xr-x)
            fs::permissions(logDir,
                            fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec |
                                fs::perms::others_read | fs::perms::others_exec,
                            fs::perm_options::replace);
        }
        m_logDir = fs::canonical(logDir);

        // Vérifier les permissions du répertoire
        auto status = fs::status(m_logDir);
        if ((status.permissions() & fs::perms::owner_write) == fs::perms::none) {
            throw std::runtime_error("No write permission for log directory: " + m_logDir.string());
        }

        // Initialiser le nom du fichier de log
        m_logFileName = getLogFileName();

        // Créer le chemin complet du fichier de log
        fs::path logFilePath = m_logDir / m_logFileName;

        // Ouvrir le fichier en mode ajout
        m_logFile.open(logFilePath.string(), std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            // Essayer de créer le fichier avec des permissions plus restrictives
            std::ofstream tempFile(logFilePath, std::ios::out);
            if (tempFile.is_open()) {
                tempFile.close();

                // Définir les permissions du fichier (rw-r-----)
                fs::permissions(
                    logFilePath,
                    fs::perms::owner_read | fs::perms::owner_write | fs::perms::group_read,
                    fs::perm_options::replace);

                // Réessayer d'ouvrir le fichier
                m_logFile.open(logFilePath.string(), std::ios::out | std::ios::app);
                if (!m_logFile.is_open()) {
                    throw std::runtime_error("Failed to open log file: " + logFilePath.string());
                }
            } else {
                throw std::runtime_error("Failed to create log file: " + logFilePath.string());
            }
        }

        if (m_logFile.is_open()) {
            m_initialized = true;
            // Écrire un en-tête dans le fichier de log
            auto now = std::chrono::system_clock::now();
            m_logFile << "\n"
                      << "========================================\n"
                      << "Open-Yolo Log - " << getTimestampString(now) << "\n"
                      << "Log Level: " << levelToString(m_minLevel) << "\n"
                      << "Thread ID: " << std::this_thread::get_id() << "\n"
                      << "========================================\n"
                      << std::endl;
        }
    } catch (const std::exception& e) {
        // En cas d'erreur, réinitialiser l'état
        m_initialized = false;
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
        throw;  // Propager l'exception
    }
}

void openyolo::Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

void openyolo::Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void openyolo::Logger::log(LogLevel level, const std::string& message, const std::string& file,
                           int line, const std::string& function) {
    if (!m_initialized || level < m_minLevel) {
        return;
    }

    Logger::LogContext context{
        file,  line,   function, std::this_thread::get_id(), std::chrono::system_clock::now(),
        level, message};

    writeLog(context);
}

void openyolo::Logger::trace(const std::string& message, const std::string& file, int line,
                             const std::string& function) {
    log(LogLevel::TRACE, message, file, line, function);
}

void openyolo::Logger::debug(const std::string& message, const std::string& file, int line,
                             const std::string& function) {
    log(LogLevel::DEBUG, message, file, line, function);
}

void openyolo::Logger::info(const std::string& message, const std::string& file, int line,
                            const std::string& function) {
    log(LogLevel::INFO, message, file, line, function);
}

void openyolo::Logger::warning(const std::string& message, const std::string& file, int line,
                               const std::string& function) {
    log(LogLevel::WARNING, message, file, line, function);
}

void openyolo::Logger::error(const std::string& message, const std::string& file, int line,
                             const std::string& function) {
    log(LogLevel::ERROR, message, file, line, function);
}

void openyolo::Logger::fatal(const std::string& message, const std::string& file, int line,
                             const std::string& function) {
    log(LogLevel::FATAL, message, file, line, function);
    // En cas d'erreur fatale, on arrête l'application
    std::exit(EXIT_FAILURE);
}

void openyolo::Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
}

void openyolo::Logger::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Marquer l'arrêt
    m_shutdownRequested = true;

    // Attendre que le thread de journalisation termine
    if (m_logThread.joinable()) {
        m_logThread.join();
    }

    // Fermer le fichier de log
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        m_logFile << "\n"
                  << "========================================\n"
                  << "Open-Yolo Log Shutdown - " << getTimestampString(now) << "\n"
                  << "========================================\n";
        m_logFile.close();
    }

    m_initialized = false;
}

void openyolo::Logger::rotateLogs() {
    if (!m_initialized) {
        return;
    }

    try {
        // Vérifier la taille du fichier actuel
        std::ifstream inFile(m_logDir / m_logFileName, std::ifstream::ate | std::ifstream::binary);
        if (inFile.is_open()) {
            std::size_t fileSize = inFile.tellg();
            inFile.close();

            if (fileSize >= m_maxFileSize) {
                // Fermer le fichier actuel
                if (m_logFile.is_open()) {
                    m_logFile.close();
                }

                // Renommer les anciens fichiers de log
                for (int i = m_maxFiles - 2; i >= 0; --i) {
                    fs::path oldFile = m_logDir / ("open-yolo.log." + std::to_string(i));
                    fs::path newFile = m_logDir / ("open-yolo.log." + std::to_string(i + 1));

                    if (fs::exists(oldFile)) {
                        if (i + 1 >= static_cast<int>(m_maxFiles) - 1) {
                            // Supprimer le fichier le plus ancien
                            fs::remove(oldFile);
                        } else {
                            // Renommer le fichier
                            fs::rename(oldFile, newFile);
                        }
                    }
                }

                // Renommer le fichier de log actuel
                fs::rename(m_logDir / m_logFileName, m_logDir / "open-yolo.log.0");

                // Rouvrir le fichier de log
                m_logFile.open(m_logDir / m_logFileName, std::ios::out | std::ios::app);

                if (m_logFile.is_open()) {
                    auto now = std::chrono::system_clock::now();
                    m_logFile << "\n"
                              << "========================================\n"
                              << "Open-Yolo Log (Rotated) - " << getTimestampString(now) << "\n"
                              << "Log Level: " << levelToString(m_minLevel) << "\n"
                              << "Thread ID: " << std::this_thread::get_id() << "\n"
                              << "========================================\n"
                              << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        // En cas d'erreur, on essaie de continuer avec le fichier actuel
        if (!m_logFile.is_open()) {
            m_logFile.open(m_logDir / m_logFileName, std::ios::out | std::ios::app);
        }
    }
}

void openyolo::Logger::writeLog(const Logger::LogContext& context) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized || m_shutdownRequested) {
        return;
    }

    try {
        // Vérifier si une rotation des logs est nécessaire
        rotateLogs();

        // Formater le message de log
        std::string logMessage = formatLogMessage(context);

        // Écrire dans le fichier
        if (m_logFile.is_open()) {
            m_logFile << logMessage << std::endl;
            m_logFile.flush();
        }

        // Écrire dans la console si activé
        if (m_consoleOutput) {
            // Utiliser des couleurs pour la console
            const char* colorCode = "";
            const char* resetCode = "\033[0m";

            switch (context.level) {
                case LogLevel::TRACE:
                    colorCode = "\033[37m";
                    break;  // Gris clair
                case LogLevel::DEBUG:
                    colorCode = "\033[36m";
                    break;  // Cyan
                case LogLevel::INFO:
                    colorCode = "\033[32m";
                    break;  // Vert
                case LogLevel::WARNING:
                    colorCode = "\033[33m";
                    break;  // Jaune
                case LogLevel::ERROR:
                    colorCode = "\033[31m";
                    break;  // Rouge
                case LogLevel::FATAL:
                    colorCode = "\033[1;31m";
                    break;  // Rouge clair
            }

            std::cerr << colorCode << logMessage << resetCode << std::endl;
        }
    } catch (const std::exception& e) {
        // En cas d'erreur d'écriture, on ne peut pas faire grand-chose
        // On essaie d'écrire dans stderr
        std::cerr << "[LOG ERROR] Failed to write log: " << e.what() << std::endl;
    }
}

std::string openyolo::Logger::levelToString(LogLevel level) const {
    static const char* const levelNames[] = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};

    return levelNames[static_cast<int>(level)];
}

std::string openyolo::Logger::getTimestampString(
    const std::chrono::system_clock::time_point& time) const {
    auto timeT = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string openyolo::Logger::formatLogMessage(const Logger::LogContext& context) const {
    std::stringstream ss;

    // Horodatage
    ss << "[" << getTimestampString(context.timestamp) << "] ";

    // Niveau de log (sur 5 caractères)
    std::string levelStr = levelToString(context.level);
    ss << std::setw(7) << std::left << ("[" + levelStr + "]") << " ";

    // Thread ID
    ss << "[" << std::hex << std::setw(8) << std::setfill('0') << context.threadId << std::dec
       << "] ";

    // Fichier et ligne (si disponibles)
    if (!context.file.empty()) {
        // Extraire uniquement le nom du fichier (sans le chemin complet)
        size_t lastSlash = context.file.find_last_of("/\\");
        std::string fileName =
            (lastSlash == std::string::npos) ? context.file : context.file.substr(lastSlash + 1);

        ss << "[" << fileName;
        if (context.line > 0) {
            ss << ":" << context.line;
        }
        ss << "] ";
    }

    // Fonction (si disponible)
    if (!context.function.empty()) {
        ss << context.function << " - ";
    }

    // Message
    ss << context.message;

    return ss.str();
}

}  // namespace openyolo
