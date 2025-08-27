#include "cursormanager/CursorManager.hpp"
#include "cursormanager/ShaderProgram.hpp"
#include <SDL2/SDL_image.h>
#include <gif_lib.h>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <unordered_map>

// Custom deleter for giflib's GifFileType
struct GifFileDeleter {
    void operator()(GifFileType* gif) {
        if (gif) {
            DGifCloseFile(gif, nullptr);
        }
    }
};

// Custom deleter for GifColorType array
struct GifColorMapDeleter {
    void operator()(GifColorType* colors) {
        delete[] colors;
    }
};

// Error handling for giflib
void GifErrorHandler() {
    // Empty handler to prevent giflib from exiting on error
}

namespace cursor_manager {

// Implémentation des destructeurs
CursorFrame::~CursorFrame() {
    if (textureId) {
        glDeleteTextures(1, &textureId);
    }
}

Framebuffer::~Framebuffer() {
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (texture) glDeleteTextures(1, &texture);
}

bool Framebuffer::create(int w, int h) {
    if (width == w && height == h && fbo != 0) {
        return true; // Déjà créé avec la bonne taille
    }
    
    width = w;
    height = h;
    
    // Créer le FBO
    if (fbo == 0) {
        glGenFramebuffers(1, &fbo);
    }
    
    // Créer la texture
    if (texture == 0) {
        glGenTextures(1, &texture);
    }
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Attacher la texture au FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    // Vérifier que le FBO est complet
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

// Implémentation du constructeur de CursorManager::Impl
CursorManager::Impl::Impl() = default;

// Implémentation du destructeur de CursorManager::Impl
CursorManager::Impl::~Impl() {
    if (window) {
        if (glContext) {
            SDL_GL_DeleteContext(glContext);
        }
        SDL_DestroyWindow(window);
    }
    
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}

bool CursorManager::Impl::initializeShaders() {
    // Vertex shader de base
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in vec2 texCoords;
        out vec2 TexCoords;
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            TexCoords = texCoords;
        }
    )";
    
    // Fragment shader pour l'effet d'ombre
    const char* shadowFragmentShader = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        uniform vec2 offset;
        uniform float alpha;
        uniform vec4 shadowColor;
        
        void main() {
            vec2 texOffset = 1.0 / textureSize(screenTexture, 0);
            vec4 result = vec4(0.0);
            
            // Appliquer un flou gaussien simple
            for(int x = -2; x <= 2; x++) {
                for(int y = -2; y <= 2; y++) {
                    vec2 samplePos = TexCoords + vec2(x, y) * texOffset * 2.0;
                    result += texture(screenTexture, samplePos) * 0.04;
                }
            }
            
            // Appliquer la couleur et l'alpha
            result = result * shadowColor * alpha;
            FragColor = result;
        }
    )";
    
    // Fragment shader pour l'effet de lueur
    const char* glowFragmentShader = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        uniform float intensity;
        uniform float size;
        uniform vec4 glowColor;
        
        void main() {
            vec2 texOffset = 1.0 / textureSize(screenTexture, 0);
            vec4 result = vec4(0.0);
            
            // Appliquer un flou gaussien plus large pour la lueur
            for(int x = -3; x <= 3; x++) {
                for(int y = -3; y <= 3; y++) {
                    float weight = exp(-(x*x + y*y) / (2.0 * size * size));
                    vec2 samplePos = TexCoords + vec2(x, y) * texOffset * 3.0;
                    result += texture(screenTexture, samplePos) * weight;
                }
            }
            
            // Normaliser et appliquer la couleur et l'intensité
            result = (result / 49.0) * glowColor * intensity * alpha;
            FragColor = result;
        }
    )";
    
    // Fragment shader final pour la composition
    const char* finalFragmentShader = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        uniform float alpha;
        
        void main() {
            vec4 texColor = texture(screenTexture, TexCoords);
            texColor.a *= alpha;
            FragColor = texColor;
        }
    )";
    
    // Charger les shaders
    shadowShader = loadShader(vertexShaderSource, shadowFragmentShader);
    glowShader = loadShader(vertexShaderSource, glowFragmentShader);
    finalShader = loadShader(vertexShaderSource, finalFragmentShader);
    
    return shadowShader && glowShader && finalShader;
}

bool CursorManager::Impl::initializeFramebuffers(int width, int height) {
    effectBuffer1 = std::make_unique<Framebuffer>();
    effectBuffer2 = std::make_unique<Framebuffer>();
    
    return effectBuffer1->create(width, height) && 
           effectBuffer2->create(width, height);
}

void CursorManager::Impl::setupScreenQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
}

void CursorManager::Impl::applyEffects(GLuint sourceTexture, int width, int height) {
    if (!effectsEnabled || effectPipeline.empty()) {
        // Aucun effet à appliquer, rendre directement
        finalShader->use();
        finalShader->setUniform("alpha", alpha);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        return;
    }
    
    // Appliquer chaque effet dans l'ordre du pipeline
    GLuint currentTexture = sourceTexture;
    
    for (const auto& effect : effectPipeline) {
        if (!effectBuffer1 || !effectBuffer2) continue;
        
        // Choisir le framebuffer de destination
        Framebuffer* destBuffer = (currentTexture == effectBuffer1->texture) ? 
                                 effectBuffer2.get() : effectBuffer1.get();
        
        if (!destBuffer->create(width, height)) continue;
        
        glBindFramebuffer(GL_FRAMEBUFFER, destBuffer->fbo);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Appliquer l'effet correspondant
        switch (effect) {
            case EffectType::Shadow:
                if (shadowSettings.enabled) {
                    applyShadowEffect(currentTexture, width, height);
                }
                break;
                
            case EffectType::Glow:
                if (glowSettings.enabled) {
                    applyGlowEffect(currentTexture, width, height);
                }
                break;
                
            default:
                break;
        }
        
        // Le résultat devient la texture source pour le prochain effet
        currentTexture = destBuffer->texture;
    }
    
    // Dernier passage pour appliquer l'alpha global
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    finalShader->use();
    finalShader->setUniform("alpha", alpha);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void CursorManager::Impl::applyShadowEffect(GLuint sourceTexture, int width, int height) {
    if (!shadowShader) return;
    
    shadowShader->use();
    shadowShader->setUniform("offset", glm::vec2(shadowSettings.offsetX / width, shadowSettings.offsetY / height));
    shadowShader->setUniform("alpha", alpha);
    shadowShader->setUniform("shadowColor", shadowSettings.color);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void CursorManager::Impl::applyGlowEffect(GLuint sourceTexture, int width, int height) {
    if (!glowShader) return;
    
    glowShader->use();
    glowShader->setUniform("intensity", glowSettings.intensity);
    glowShader->setUniform("size", glowSettings.size);
    glowShader->setUniform("glowColor", glowSettings.color);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

std::unique_ptr<ShaderProgram> CursorManager::Impl::loadShader(
    const std::string& vertexSrc, 
    const std::string& fragmentSrc) 
{
    // Créer des fichiers temporaires pour les shaders
    std::string vertexFile = "/tmp/shader_vert.glsl";
    std::string fragmentFile = "/tmp/shader_frag.glsl";
    
    // Écrire les sources dans des fichiers temporaires
    std::ofstream vertFile(vertexFile);
    vertFile << vertexSrc;
    vertFile.close();
    
    std::ofstream fragFile(fragmentFile);
    fragFile << fragmentSrc;
    fragFile.close();
    
    // Charger les shaders depuis les fichiers
    auto shader = std::make_unique<ShaderProgram>();
    if (!shader->loadFromFiles(vertexFile, fragmentFile)) {
        std::cerr << "Failed to load shader from files" << std::endl;
        // Supprimer les fichiers temporaires
        std::remove(vertexFile.c_str());
        std::remove(fragmentFile.c_str());
        return nullptr;
    }
    
    // Supprimer les fichiers temporaires
    std::remove(vertexFile.c_str());
    std::remove(fragmentFile.c_str());
    
    return shader;
}

bool CursorManager::Impl::initialize() {
    if (isInitialized) return true;
    
    // Initialiser SDL et OpenGL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Configuration d'OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // Créer une fenêtre cachée pour le contexte OpenGL
    window = SDL_CreateWindow("CursorManager", 
                             SDL_WINDOWPOS_UNDEFINED, 
                             SDL_WINDOWPOS_UNDEFINED, 
                             1, 1, 
                             SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Créer le contexte OpenGL
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialiser GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
        return false;
    }
    
    // Configurer le viewport initial
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);
    
    // Configurer les états OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialiser les shaders et les buffers
    if (!initializeShaders()) {
        std::cerr << "Failed to initialize shaders" << std::endl;
        return false;
    }
    
    if (!initializeFramebuffers(w, h)) {
        std::cerr << "Failed to initialize framebuffers" << std::endl;
        return false;
    }
    
    setupScreenQuad();
    
    // Initialiser le temps de la dernière frame
    lastFrameTime = std::chrono::steady_clock::now();
    
    isInitialized = true;
    return true;
}

void CursorManager::Impl::update() {
    if (cursorFrames.empty() || cursorFrames.size() == 1) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastFrameTime).count();
    
    if (elapsed >= cursorFrames[currentFrame]->delayMs) {
        currentFrame = (currentFrame + 1) % cursorFrames.size();
        lastFrameTime = now;
    }
}

void CursorManager::Impl::render() {
    if (!isInitialized || !isVisible || cursorFrames.empty()) return;
    
    // Sauvegarder l'état OpenGL actuel
    GLint lastFBO, lastViewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    
    // Obtenir la taille de la fenêtre
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    
    // Mettre à jour la taille des framebuffers si nécessaire
    if (w != effectBuffer1->width || h != effectBuffer1->height) {
        initializeFramebuffers(w, h);
    }
    
    // Rendre le curseur avec les effets
    const auto& frame = cursorFrames[currentFrame];
    applyEffects(frame->textureId, frame->width, frame->height);
    
    // Restaurer l'état OpenGL
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    glViewport(lastViewport[0], lastViewport[1], 
              (GLsizei)lastViewport[2], (GLsizei)lastViewport[3]);
}

void CursorManager::Impl::loadCursor(const std::string& path) {
    // Implémentation simplifiée - à compléter avec le chargement d'images/GIF
    // Cette méthode devrait charger les frames du curseur et les stocker dans cursorFrames
    
    // Exemple de code pour charger une image simple (à adapter pour les GIF)
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load cursor image: " << IMG_GetError() << std::endl;
        return;
    }
    
    // Convertir au format RGBA si nécessaire
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    
    if (!converted) {
        std::cerr << "Failed to convert cursor image" << std::endl;
        return;
    }
    
    // Créer une texture OpenGL
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Configurer les paramètres de texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Télécharger les données de l'image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted->w, converted->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);
    
    // Créer une frame de curseur
    auto frame = std::make_unique<CursorFrame>();
    frame->textureId = textureId;
    frame->width = converted->w;
    frame->height = converted->h;
    frame->hasTransparency = true; // À vérifier selon l'image
    
    // Ajouter la frame à la liste
    cursorFrames.clear(); // Supprimer les anciennes frames
    cursorFrames.push_back(std::move(frame));
    currentFrame = 0;
    
    // Libérer la surface
    SDL_FreeSurface(converted);
}

void CursorManager::Impl::setEffectEnabled(const std::string& effectName, bool enabled) {
    if (effectName == "shadow") {
        shadowSettings.enabled = enabled;
    } else if (effectName == "glow") {
        glowSettings.enabled = enabled;
    }
    
    // Mettre à jour le pipeline d'effets si nécessaire
    auto it = std::find(effectPipeline.begin(), effectPipeline.end(),
                       (effectName == "shadow") ? EffectType::Shadow : EffectType::Glow);
    
    if (enabled && it == effectPipeline.end()) {
        effectPipeline.push_back((effectName == "shadow") ? EffectType::Shadow : EffectType::Glow);
    } else if (!enabled && it != effectPipeline.end()) {
        effectPipeline.erase(it);
    }
}

bool CursorManager::Impl::isEffectEnabled(const std::string& effectName) const {
    if (effectName == "shadow") {
        return shadowSettings.enabled;
    } else if (effectName == "glow") {
        return glowSettings.enabled;
    }
    return false;
}

void CursorManager::Impl::setEffectPipeline(const std::vector<std::string>& effects) {
    effectPipeline.clear();
    
    for (const auto& effect : effects) {
        if (effect == "shadow") {
            effectPipeline.push_back(EffectType::Shadow);
        } else if (effect == "glow") {
            effectPipeline.push_back(EffectType::Glow);
        }
    }
}

void CursorManager::Impl::setShadowEffect(int offsetX, int offsetY, float blurRadius, 
                                         const std::array<float,4>& color) {
    shadowSettings.offsetX = static_cast<float>(offsetX);
    shadowSettings.offsetY = static_cast<float>(offsetY);
    shadowSettings.blurRadius = blurRadius;
    shadowSettings.color = glm::vec4(color[0], color[1], color[2], color[3]);
}

void CursorManager::Impl::optimizeGPUMemory() {
    // Libérer les ressources GPU inutilisées
    if (effectBuffer1) {
        effectBuffer1->texture = 0;
        effectBuffer1->fbo = 0;
    }
    
    if (effectBuffer2) {
        effectBuffer2->texture = 0;
        effectBuffer2->fbo = 0;
    }
    
    // Réinitialiser les shaders
    shadowShader.reset();
    glowShader.reset();
    finalShader.reset();
    
    // Réinitialiser les états
    isInitialized = false;
}

void CursorManager::Impl::clearEffects() {
    shadowSettings.enabled = false;
    glowSettings.enabled = false;
    effectPipeline.clear();
    
    // Réinitialiser les paramètres par défaut
    shadowSettings = ShadowSettings();
    glowSettings = GlowSettings();
    
    // Réactiver les effets de base
    effectPipeline = {EffectType::Shadow, EffectType::Glow};
}

// Implémentation des méthodes de CursorManager
CursorManager::CursorManager() : pImpl(std::make_unique<Impl>()) {}
CursorManager::~CursorManager() = default;

bool CursorManager::initialize() { return pImpl->initialize(); }
void CursorManager::update() { pImpl->update(); }
void CursorManager::render() { pImpl->render(); }
void CursorManager::setVisible(bool visible) { pImpl->setVisible(visible); }
void CursorManager::setFps(int fps) { pImpl->setFps(fps); }
void CursorManager::setAlpha(float alpha) { pImpl->setAlpha(alpha); }
void CursorManager::setEffectsEnabled(bool enabled) { pImpl->setEffectsEnabled(enabled); }
void CursorManager::setEffectEnabled(const std::string& effectName, bool enabled) { 
    pImpl->setEffectEnabled(effectName, enabled); 
}
bool CursorManager::isEffectEnabled(const std::string& effectName) const { 
    return pImpl->isEffectEnabled(effectName); 
}
void CursorManager::setEffectPipeline(const std::vector<std::string>& effects) { 
    pImpl->setEffectPipeline(effects); 
}
void CursorManager::setShadowEffect(int offsetX, int offsetY, float blurRadius, 
                                   const std::array<float,4>& color) {
    pImpl->setShadowEffect(offsetX, offsetY, blurRadius, color);
}
void CursorManager::setCursor(const std::string& cursorPath) { pImpl->loadCursor(cursorPath); }
void CursorManager::loadFromFile(const std::string& path) {
    pImpl->loadCursor(path);
}

void CursorManager::setScale(float scale) {
    // Implémentation de base - à adapter selon les besoins
    // On pourrait ajouter une méthode dans Impl pour gérer le changement d'échelle
    // Pour l'instant, on recharge simplement le curseur actuel avec la nouvelle échelle
    if (!pImpl) return;
    
    // On pourrait stocker le chemin actuel et le recharger
    // Mais comme on n'a pas accès direct au chemin, on ne fait rien pour l'instant
    // Une meilleure implémentation serait d'ajouter une méthode getCurrentCursorPath()
    
    // On peut quand même mettre à jour la variable d'échelle si elle existe
    pImpl->setScale(scale);
}

void CursorManager::setAnimationSpeed(int fps) {
    if (pImpl && fps > 0) {
        pImpl->setFps(fps);
    }
}
void CursorManager::optimizeGPUMemory() { pImpl->optimizeGPUMemory(); }
void CursorManager::clearEffects() { pImpl->clearEffects(); }

} // namespace cursor_manager
