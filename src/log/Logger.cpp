#include "log/Logger.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

namespace openyolo {

// Implémentation du singleton
Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

// Constructeur privé
Logger::Logger() 
    : m_maxFileSize(5 * 1024 * 1024) // 5 Mo par défaut
    , m_maxFiles(10)
    , m_minLevel(LogLevel::INFO)
    , m_consoleOutput(true)
    , m_initialized(false)
    , m_shutdownRequested(false) {
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
Logger::~Logger() {
    shutdown();
}

void Logger::initialize(const fs::path& logDir, size_t maxFileSize, size_t maxFiles, 
                       LogLevel minLevel, bool consoleOutput) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return; // Déjà initialisé
    }

    // Mettre à jour les paramètres
    if (!logDir.empty()) {
        m_logDir = logDir;
    }
    m_maxFileSize = maxFileSize;
    m_maxFiles = maxFiles;
    m_minLevel = minLevel;
    m_consoleOutput = consoleOutput;
    m_shutdownRequested = false;

    // Créer le répertoire de logs s'il n'existe pas
    try {
        if (!fs::exists(m_logDir)) {
            fs::create_directories(m_logDir);
            // Définir les permissions du répertoire (rwxr-x---)
            fs::permissions(m_logDir, 
                           fs::perms::owner_all | 
                           fs::perms::group_read | fs::perms::group_exec |
                           fs::perms::others_none);
        }
    } catch (const std::exception& e) {
        // En cas d'erreur, on essaie d'utiliser /tmp
        m_logDir = "/tmp/open-yolo/logs";
        try {
            fs::create_directories(m_logDir);
        } catch (...) {
            // Si on ne peut pas créer le répertoire, on ne peut pas logger
            return;
        }
    }

    // Ouvrir le fichier de log
    m_logFileName = getLogFileName();
    fs::path logFilePath = m_logDir / m_logFileName;
    
    try {
        m_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            // Essayer de créer le fichier avec des permissions plus restrictives
            std::ofstream tempFile(logFilePath, std::ios::out);
            if (tempFile.is_open()) {
                tempFile.close();
                // Chmod 640 (rw-r-----)
                fs::permissions(logFilePath, 
                               fs::perms::owner_read | fs::perms::owner_write |
                               fs::perms::group_read |
                               fs::perms::others_none);
                m_logFile.open(logFilePath, std::ios::out | std::ios::app);
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
        // Ne pas lever d'exception dans le constructeur
        m_initialized = false;
    }
}

void Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void Logger::log(LogLevel level, const std::string& message, 
                const std::string& file, int line, const std::string& function) {
    if (!m_initialized || level < m_minLevel) {
        return;
    }

    LogContext context{
        file,
        line,
        function,
        std::chrono::system_clock::now(),
        level,
        message,
        std::this_thread::get_id()
    };

    writeLog(context);
}

void Logger::trace(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::TRACE, message, file, line, function);
}

void Logger::debug(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::DEBUG, message, file, line, function);
}

void Logger::info(const std::string& message, const std::string& file, 
                 int line, const std::string& function) {
    log(LogLevel::INFO, message, file, line, function);
}

void Logger::warning(const std::string& message, const std::string& file, 
                    int line, const std::string& function) {
    log(LogLevel::WARNING, message, file, line, function);
}

void Logger::error(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::ERROR, message, file, line, function);
}

void Logger::fatal(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::FATAL, message, file, line, function);
    // En cas d'erreur fatale, on arrête l'application
    std::exit(EXIT_FAILURE);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
}

void Logger::shutdown() {
    if (m_shutdownRequested) {
        return;
    }
    
    m_shutdownRequested = true;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        m_logFile << "\n"
                 << "========================================\n"
                 << "Open-Yolo Log Shutdown - " << getTimestampString(now) << "\n"
                 << "========================================\n"
                 << std::endl;
        m_logFile.close();
    }
    m_initialized = false;
}

void Logger::rotateLogs() {
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

void Logger::writeLog(const LogContext& context) {
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
                case LogLevel::TRACE:   colorCode = "\033[37m"; break; // Gris clair
                case LogLevel::DEBUG:   colorCode = "\033[36m"; break; // Cyan
                case LogLevel::INFO:    colorCode = "\033[32m"; break; // Vert
                case LogLevel::WARNING: colorCode = "\033[33m"; break; // Jaune
                case LogLevel::ERROR:   colorCode = "\033[31m"; break; // Rouge
                case LogLevel::FATAL:   colorCode = "\033[1;31m"; break; // Rouge clair
            }
            
            std::cerr << colorCode << logMessage << resetCode << std::endl;
        }
    } catch (const std::exception& e) {
        // En cas d'erreur d'écriture, on ne peut pas faire grand-chose
        // On essaie d'écrire dans stderr
        std::cerr << "[LOG ERROR] Failed to write log: " << e.what() << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    static const char* const levelNames[] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "FATAL"
    };
    
    return levelNames[static_cast<int>(level)];
}

std::string Logger::getTimestampString(const std::chrono::system_clock::time_point& time) const {
    auto timeT = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::formatLogMessage(const LogContext& context) const {
    std::stringstream ss;
    
    // Horodatage
    ss << "[" << getTimestampString(context.timestamp) << "] ";
    
    // Niveau de log (sur 5 caractères)
    std::string levelStr = levelToString(context.level);
    ss << std::setw(7) << std::left << ("[" + levelStr + "]") << " ";
    
    // Thread ID
    ss << "[" << std::hex << std::setw(8) << std::setfill('0') 
       << context.threadId << std::dec << "] ";
    
    // Fichier et ligne (si disponibles)
    if (!context.file.empty()) {
        // Extraire uniquement le nom du fichier (sans le chemin complet)
        size_t lastSlash = context.file.find_last_of("/\\");
        std::string fileName = (lastSlash == std::string::npos) 
            ? context.file 
            : context.file.substr(lastSlash + 1);
        
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

std::string Logger::getLogFileName() const {
    return "open-yolo.log";
}

} // namespace openyolo
