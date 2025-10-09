#include "Logger.hpp"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <iostream>

namespace openyolo {

void Logger::initialize(const std::string& filename, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    
    m_logFile.open(filename, std::ios::out | std::ios::trunc);
    m_minLevel = minLevel;
    
    if (!m_logFile.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier de log: " + filename);
    }
    
    // Écrire l'en-tête du fichier de log
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    m_logFile << "=== Open-Yolo - Démarrage du journal " 
              << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") 
              << " ===\n";
    m_logFile << "Niveau de log minimum: " << levelToString(minLevel) << "\n\n";
    m_logFile.flush();
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile << "\n=== Fin du journal ===\n";
        m_logFile.close();
    }
}

void Logger::log(LogLevel level, const std::string& message, 
                const std::string& file, int line) {
    if (level < m_minLevel) {
        return; // Ignorer les messages de niveau inférieur au niveau minimum
    }
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";
    
    ss << "[" << levelToString(level) << "] ";
    
    if (!file.empty()) {
        // Extraire uniquement le nom du fichier (sans le chemin)
        size_t pos = file.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? file.substr(pos + 1) : file;
        ss << filename;
        if (line > 0) {
            ss << ":" << line;
        }
        ss << " - ";
    }
    
    ss << message << "\n";
    
    // Écrire dans le fichier de manière thread-safe
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_logFile.is_open()) {
            m_logFile << ss.str() << std::flush;
        }
    }
    
    // Écrire également sur la sortie standard pour les erreurs et les messages importants
    if (level >= LogLevel::WARNING) {
        std::cerr << ss.str() << std::flush;
    } else if (level == LogLevel::INFO) {
        std::cout << ss.str() << std::flush;
    }
}

const char* Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE:   return "TRACE";
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNW";
    }
}

} // namespace openyolo
