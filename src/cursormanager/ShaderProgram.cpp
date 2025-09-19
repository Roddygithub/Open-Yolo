#include "include/cursormanager/ShaderProgram.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram() {
    m_programId = glCreateProgram();
    if (!m_programId) {
        throw std::runtime_error("Failed to create shader program");
    }
}

ShaderProgram::~ShaderProgram() {
    if (m_programId) {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
}

bool ShaderProgram::loadFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    if (m_programId == 0) {
        std::cerr << "Shader program not properly initialized" << std::endl;
        return false;
    }

    // Charger les sources des shaders
    std::string vertexSource = loadShaderSource(vertexShaderPath);
    std::string fragmentSource = loadShaderSource(fragmentShaderPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to load shader sources" << std::endl;
        return false;
    }
    
    // Compiler les shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) {
        std::cerr << "Failed to compile vertex shader" << std::endl;
        return false;
    }
    
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        std::cerr << "Failed to compile fragment shader" << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Détacher les shaders existants
    GLint numShaders = 0;
    glGetProgramiv(m_programId, GL_ATTACHED_SHADERS, &numShaders);
    if (numShaders > 0) {
        std::vector<GLuint> shaders(numShaders);
        glGetAttachedShaders(m_programId, numShaders, nullptr, shaders.data());
        for (GLuint shader : shaders) {
            glDetachShader(m_programId, shader);
            glDeleteShader(shader);
        }
    }
    
    // Attacher les nouveaux shaders
    glAttachShader(m_programId, vertexShader);
    glAttachShader(m_programId, fragmentShader);
    
    // Lier le programme
    glLinkProgram(m_programId);
    
    // Vérifier les erreurs de linkage
    GLint success;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        
        // Nettoyer
        glDetachShader(m_programId, vertexShader);
        glDetachShader(m_programId, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return false;
    }
    
    // Nettoyer les shaders après le linkage réussi
    glDetachShader(m_programId, vertexShader);
    glDetachShader(m_programId, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Vider le cache des uniformes
    m_uniformLocations.clear();
    
    // Valider le programme
    glValidateProgram(m_programId);
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
        std::cerr << "Shader program validation failed: " << infoLog << std::endl;
        return false;
    }
    
    // Vérifier le programme
    glValidateProgram(m_programId);
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
        std::cerr << "Shader program validation failed: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

void ShaderProgram::bind() const {
    if (m_programId) {
        glUseProgram(m_programId);
    }
}

void ShaderProgram::unbind() const {
    glUseProgram(0);
}

void ShaderProgram::setUniform(const std::string& name, int value) {
    if (m_programId) {
        glUniform1i(getUniformLocation(name), value);
    }
}

void ShaderProgram::setUniform(const std::string& name, float value) {
    if (m_programId) {
        glUniform1f(getUniformLocation(name), value);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec2& value) {
    if (m_programId) {
        glUniform2f(getUniformLocation(name), value.x, value.y);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) {
    if (m_programId) {
        glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4& value) {
    if (m_programId) {
        glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& value) {
    if (m_programId) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }
}

GLint ShaderProgram::getUniformLocation(const std::string& name) {
    if (!m_programId) {
        return -1;
    }
    
    // Vérifier le cache
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }
    
    // Obtenir l'emplacement de l'uniforme
    GLint location = glGetUniformLocation(m_programId, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
    }
    
    // Mettre en cache le résultat
    m_uniformLocations[name] = location;
    return location;
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source) {
    if (source.empty()) {
        std::cerr << "Shader source is empty" << std::endl;
        return 0;
    }
    
    GLuint shader = glCreateShader(type);
    if (!shader) {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }
    
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    // Vérifier les erreurs de compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        
        std::cerr << "Shader compilation failed: " << log.data() << std::endl;
        
        // Afficher la source du shader pour le débogage
        std::cerr << "Shader source:" << std::endl;
        std::istringstream stream(source);
        std::string line;
        int lineNum = 1;
        while (std::getline(stream, line)) {
            std::cerr << lineNum << ": " << line << std::endl;
            lineNum++;
        }
        
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

std::string ShaderProgram::loadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    
    try {
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        // Vérifier que le fichier n'est pas vide
        std::string source = buffer.str();
        if (source.empty()) {
            std::cerr << "Shader file is empty: " << filePath << std::endl;
            return "";
        }
        
        return source;
    } catch (const std::exception& e) {
        std::cerr << "Error reading shader file " << filePath << ": " << e.what() << std::endl;
        return "";
    }
}
