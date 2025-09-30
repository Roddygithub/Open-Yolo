#include <gtest/gtest.h>
#include "cursormanager/CursorManager.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <filesystem>

using namespace cursor_manager;

// Fixture pour les tests du CursorManager
class CursorManagerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Initialisation SDL une seule fois pour tous les tests
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            FAIL() << "Échec de l'initialisation SDL: " << SDL_GetError();
        }
        
        // Initialisation de SDL_image
        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
            FAIL() << "Échec de l'initialisation SDL_image: " << IMG_GetError();
        }
    }
    
    static void TearDownTestSuite() {
        IMG_Quit();
        SDL_Quit();
    }
    
    void SetUp() override {
        // Création d'une fenêtre SDL pour le contexte OpenGL
        window = SDL_CreateWindow("Test CursorManager", 
                                 SDL_WINDOWPOS_UNDEFINED, 
                                 SDL_WINDOWPOS_UNDEFINED,
                                 100, 100,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (!window) {
            FAIL() << "Échec de la création de la fenêtre SDL: " << SDL_GetError();
        }
        
        // Création du contexte OpenGL
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            FAIL() << "Échec de la création du contexte OpenGL: " << SDL_GetError();
        }
        
        // Initialisation de GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            FAIL() << "Échec de l'initialisation GLEW: " << glewGetErrorString(glewError);
        }
        
        // Création d'un répertoire temporaire pour les tests
        testDir = std::filesystem::temp_directory_path() / "open-yolo-test";
        std::filesystem::create_directories(testDir);
    }
    
    void TearDown() override {
        // Nettoyage
        if (glContext) {
            SDL_GL_DeleteContext(glContext);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        
        // Suppression du répertoire de test
        std::filesystem::remove_all(testDir);
    }
    
    // Crée un fichier image de test
    std::string createTestImage(const std::string& filename) {
        std::string path = (testDir / filename).string();
        
        // Création d'une surface SDL simple (1x1 pixel)
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
            0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);
        
        if (!surface) {
            ADD_FAILURE() << "Échec de la création de la surface de test";
            return "";
        }
        
        // Sauvegarde en PNG
        if (filename.ends_with(".png")) {
            IMG_SavePNG(surface, path.c_str());
        } 
        // Autres formats d'image si nécessaire...
        
        SDL_FreeSurface(surface);
        return path;
    }
    
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    std::filesystem::path testDir;
};

// Test d'initialisation de base
TEST_F(CursorManagerTest, Initialization) {
    CursorManager manager;
    EXPECT_TRUE(manager.initialize());
}

// Test de chargement d'un curseur statique
TEST_F(CursorManagerTest, LoadStaticCursor) {
    std::string testImage = createTestImage("test_cursor.png");
    ASSERT_FALSE(testImage.empty()) << "Échec de la création de l'image de test";
    
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Test avec un chemin valide
    EXPECT_NO_THROW(manager.setCursor(testImage));
    
    // Test avec un chemin invalide - ne lance pas d'exception, affiche juste une erreur
    EXPECT_NO_THROW(manager.setCursor("fichier_inexistant.png"));
}

// Test de configuration du FPS
TEST_F(CursorManagerTest, SetFPS) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Test avec des valeurs valides
    EXPECT_NO_THROW(manager.setAnimationSpeed(30));
    EXPECT_NO_THROW(manager.setAnimationSpeed(60));
    
    // Test avec des valeurs limites
    EXPECT_NO_THROW(manager.setAnimationSpeed(1));
    EXPECT_NO_THROW(manager.setAnimationSpeed(144));
    
    // Test avec des valeurs invalides (doivent être clampées)
    EXPECT_NO_THROW(manager.setAnimationSpeed(-10));
    EXPECT_NO_THROW(manager.setAnimationSpeed(1000));
}

// Test de visibilité du curseur
TEST_F(CursorManagerTest, Visibility) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Par défaut, le curseur devrait être visible
    EXPECT_NO_THROW(manager.setVisible(true));
    EXPECT_NO_THROW(manager.setVisible(false));
}

// Test de mise à l'échelle
TEST_F(CursorManagerTest, Scaling) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Test avec des échelles valides
    EXPECT_NO_THROW(manager.setScale(1.0f));
    EXPECT_NO_THROW(manager.setScale(2.5f));
    
    // Test avec des valeurs limites
    EXPECT_NO_THROW(manager.setScale(0.1f));
    EXPECT_NO_THROW(manager.setScale(5.0f));
    
    // Test avec des valeurs invalides (doivent être clampées)
    EXPECT_NO_THROW(manager.setScale(-1.0f));
    EXPECT_NO_THROW(manager.setScale(10.0f));
}

// Test de mise à jour du curseur
TEST_F(CursorManagerTest, Update) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // La mise à jour ne devrait pas planter même sans curseur chargé
    EXPECT_NO_THROW(manager.update());
    
    // Chargement d'un curseur de test
    std::string testImage = createTestImage("test_update.png");
    ASSERT_FALSE(testImage.empty());
    
    EXPECT_NO_THROW(manager.setCursor(testImage));
    
    // La mise à jour avec un curseur chargé ne devrait pas planter
    EXPECT_NO_THROW(manager.update());
}

// Test de performance pour vérifier que le rendu est bien accéléré par le GPU
TEST_F(CursorManagerTest, Performance) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Création d'une image de test
    std::string testImage = createTestImage("test_perf.png");
    ASSERT_FALSE(testImage.empty());
    
    // Chargement du curseur
    EXPECT_NO_THROW(manager.setCursor(testImage));
    
    // Mesure du temps de rendu pour 1000 frames
    const int frameCount = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < frameCount; ++i) {
        manager.update();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Le rendu devrait être suffisamment rapide pour du GPU
    // Moins de 100ms pour 1000 frames (soit 10 000 FPS théorique)
    EXPECT_LT(duration.count(), 100) << "Les performances de rendu sont trop faibles";
}

// Test de l'effet d'ombre
TEST_F(CursorManagerTest, ShadowEffect) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Configuration de l'effet d'ombre
    manager.setShadowEffect(3, 3, 5.0f, {0.0f, 0.0f, 0.0f, 0.5f});
    
    // Test de rendu avec l'effet
    manager.update();
    manager.render();
    
    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}

// Test de l'effet de lueur
TEST_F(CursorManagerTest, GlowEffect) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Configuration du pipeline d'effets pour n'utiliser que la lueur
    std::vector<std::string> effects = {"glow"};
    manager.setEffectPipeline(effects);
    
    // Test de rendu avec l'effet
    manager.update();
    manager.render();
    
    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}

// Test du pipeline d'effets combinés
TEST_F(CursorManagerTest, EffectPipeline) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Configuration de l'effet d'ombre
    manager.setShadowEffect(2, 2, 3.0f, {0.0f, 0.0f, 0.0f, 0.5f});
    
    // Configuration du pipeline d'effets
    std::vector<std::string> effects = {"shadow", "glow"};
    manager.setEffectPipeline(effects);
    
    // Test de rendu avec les effets
    manager.update();
    manager.render();
    
    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}

// Test de la méthode clearEffects
TEST_F(CursorManagerTest, ClearEffects) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Configuration de l'effet d'ombre
    manager.setShadowEffect(2, 2, 3.0f, {0.0f, 0.0f, 0.0f, 0.5f});
    
    // Configuration du pipeline d'effets
    std::vector<std::string> effects = {"shadow", "glow"};
    manager.setEffectPipeline(effects);
    
    // Réinitialisation des effets
    manager.clearEffects();
    
    // Test de rendu après réinitialisation
    manager.update();
    manager.render();
    
    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}

// Test de configuration du pipeline d'effets
TEST_F(CursorManagerTest, EffectPipelineConfiguration) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());
    
    // Configuration de l'effet d'ombre
    manager.setShadowEffect(2, 2, 3.0f, {0.0f, 0.0f, 0.0f, 0.5f});
    
    // Test avec un pipeline vide (devrait utiliser le rendu de base)
    manager.setEffectPipeline({});
    manager.update();
    manager.render();
    
    // Test avec un pipeline contenant un effet inconnu (devrait être ignoré)
    manager.setEffectPipeline({"unknown_effect"});
    manager.update();
    manager.render();
    
    // Test avec un pipeline valide
    manager.setEffectPipeline({"shadow"});
    manager.update();
    manager.render();
    
    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}
