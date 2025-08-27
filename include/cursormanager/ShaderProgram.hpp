#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    // Chargement et compilation des shaders
    bool loadFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    
    // Gestion du programme shader
    void bind() const;
    void unbind() const;
    
    // Getters
    GLuint getProgramId() const { return m_programId; }
    
    // Méthodes pour définir les uniformes
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const glm::vec2& value);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::vec4& value);
    void setUniform(const std::string& name, const glm::mat4& value);
    
    // Méthodes dépréciées (pour compatibilité)
    void use() const { bind(); }
    void setUniform1i(const std::string& name, int value) { setUniform(name, value); }
    void setUniform1f(const std::string& name, float value) { setUniform(name, value); }
    void setUniform2f(const std::string& name, float v0, float v1) { setUniform(name, glm::vec2(v0, v1)); }
    void setUniform3f(const std::string& name, float v0, float v1, float v2) { setUniform(name, glm::vec3(v0, v1, v2)); }
    void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) { 
        setUniform(name, glm::vec4(v0, v1, v2, v3)); 
    }
    void setUniformMatrix4fv(const std::string& name, const float* value) { 
        setUniform(name, glm::mat4(glm::make_mat4(value))); 
    }

private:
    GLuint m_programId = 0;
    std::unordered_map<std::string, GLint> m_uniformLocations;
    
    // Méthodes utilitaires
    GLuint compileShader(GLenum type, const std::string& source);
    std::string loadShaderSource(const std::string& filePath);
    GLint getUniformLocation(const std::string& name);
};
