#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include <algorithm>

// Forward declarations
class ShaderProgram;

namespace cursor_manager {

// Structure pour stocker les données d'une frame de curseur
struct CursorFrame {
    GLuint textureId = 0;
    int width = 0;
    int height = 0;
    int delayMs = 10; // Délai en millisecondes
    bool hasTransparency = false;
    int hotspotX = 0;
    int hotspotY = 0;
    
    ~CursorFrame();
};

// Structure pour les paramètres d'ombre
struct ShadowSettings {
    bool enabled = true;
    float offsetX = 2.0f;
    float offsetY = 2.0f;
    float blurRadius = 3.0f;
    glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.7f};
};

// Structure pour les paramètres de lueur
struct GlowSettings {
    bool enabled = false;
    float size = 10.0f;
    float intensity = 0.5f;
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
};

// Structure pour un framebuffer
struct Framebuffer {
    GLuint fbo = 0;
    GLuint texture = 0;
    int width = 0;
    int height = 0;
    
    ~Framebuffer();
    bool create(int w, int h);
};

class CursorManager {
public:
    // Types d'effets disponibles
    enum class EffectType {
        NoEffect,  // Renommé de None à NoEffect pour éviter le conflit avec X11/X.h
        Shadow,
        Glow
    };

    CursorManager();
    ~CursorManager();

    // Initialisation et gestion du cycle de vie
    bool initialize();
    void update();
    void render();
    void setVisible(bool visible);
    void setFps(int fps);

    // Gestion des effets
    void setAlpha(float alpha);
    void setEffectsEnabled(bool enabled);
    void setEffectEnabled(const std::string& effectName, bool enabled);
    bool isEffectEnabled(const std::string& effectName) const;
    void setEffectPipeline(const std::vector<std::string>& effects);
    void setShadowEffect(int offsetX, int offsetY, float blurRadius = 3.0f, 
                        const std::array<float,4>& color = {0.0f, 0.0f, 0.0f, 0.7f});
    
    // Gestion des curseurs
    void setCursor(const std::string& cursorPath);
    void loadFromFile(const std::string& path);
    
    // Gestion de l'échelle et de l'animation
    void setScale(float scale);
    void setSize(float size);  // Alias pour setScale
    void setAnimationSpeed(int fps);
    
    // Activation/désactivation du curseur personnalisé
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    // Optimisation
    void optimizeGPUMemory();
    void clearEffects();
    
    // Méthode de création
    static std::unique_ptr<CursorManager> create() {
        return std::unique_ptr<CursorManager>(new CursorManager());
    }

private:
    // Implémentation détaillée (PIMPL)
    class Impl {
    public:
        Impl();
        ~Impl();

        // Méthodes publiques
        bool initialize();
        void update();
        void render();
        void setVisible(bool visible) { isVisible = visible; }
        void setFps(int fps) { 
            if (fps > 0) targetFps = fps; 
        }
        void setAlpha(float a) { alpha = (a < 0.0f) ? 0.0f : (a > 1.0f) ? 1.0f : a; }
        void setEffectsEnabled(bool enabled) { effectsEnabled = enabled; }
        void loadCursor(const std::string& path);
        void optimizeGPUMemory();
        void clearEffects();
        
        // Gestion des effets
        void setEffectEnabled(const std::string& effectName, bool enabled);
        bool isEffectEnabled(const std::string& effectName) const;
        void setEffectPipeline(const std::vector<std::string>& effects);
        void setShadowEffect(int offsetX, int offsetY, float blurRadius, 
                           const std::array<float,4>& color);
        
        // Gestion de l'échelle
        void setScale(float scale) { scale_ = scale; }
        void setEnabled(bool enabled) { isVisible = enabled; }
        bool isEnabled() const { return isVisible; }

    private:
        // Méthodes privées
        bool initializeShaders();
        bool initializeFramebuffers(int width, int height);
        void setupScreenQuad();
        void applyEffects(GLuint sourceTexture, int width, int height);
        void applyShadowEffect(GLuint sourceTexture, int width, int height);
        void applyGlowEffect(GLuint sourceTexture, int width, int height);
        
        std::unique_ptr<ShaderProgram> loadShader(
            const std::string& vertexSrc,
            const std::string& fragmentSrc
        );
        
        // Membres
        SDL_Window* window = nullptr;
        SDL_GLContext glContext = nullptr;
        
        // Shaders
        std::unique_ptr<ShaderProgram> shadowShader;
        std::unique_ptr<ShaderProgram> glowShader;
        std::unique_ptr<ShaderProgram> finalShader;
        
        // Framebuffers pour les effets
        std::unique_ptr<Framebuffer> effectBuffer1;
        std::unique_ptr<Framebuffer> effectBuffer2;
        
        // Paramètres d'effets
        ShadowSettings shadowSettings;
        GlowSettings glowSettings;
        
        // Données du curseur
        std::vector<std::unique_ptr<CursorFrame>> cursorFrames;
        size_t currentFrame = 0;
        std::chrono::time_point<std::chrono::steady_clock> lastFrameTime;
        
        // État
        bool isInitialized = false;
        bool isVisible = true;
        float alpha = 1.0f;
        float scale_ = 1.0f;
        bool effectsEnabled = true;
        int targetFps = 60;
        std::vector<EffectType> effectPipeline = {EffectType::Shadow, EffectType::Glow};
        
        // Quad pour le rendu
        GLuint quadVAO = 0;
        GLuint quadVBO = 0;
    };
    
    std::unique_ptr<Impl> pImpl;
};

} // namespace cursor_manager
