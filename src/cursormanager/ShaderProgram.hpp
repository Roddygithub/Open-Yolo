#pragma once

#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();
    
    bool loadFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    void bind() const;
    void unbind() const;
    
    // Uniform setters
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const glm::vec2& value);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::vec4& value);
    void setUniform(const std::string& name, const glm::mat4& value);
    
private:
    GLuint m_programId;
    std::unordered_map<std::string, GLint> m_uniformLocations;
    
    GLint getUniformLocation(const std::string& name);
    GLuint compileShader(GLenum type, const std::string& source);
    std::string loadShaderSource(const std::string& filePath);
};
