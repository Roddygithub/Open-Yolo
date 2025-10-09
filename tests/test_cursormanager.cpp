#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <gtest/gtest.h>

#include "cursormanager/CursorManager.hpp"
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <chrono>
#include <filesystem>
#include <thread>

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
        window =
            SDL_CreateWindow("Test CursorManager", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
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

        // Vérification de la version d'OpenGL
        const GLubyte* version = glGetString(GL_VERSION);
        if (!version) {
            FAIL() << "Impossible de récupérer la version d'OpenGL";
        }
        std::cout << "OpenGL version: " << version << std::endl;

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
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);

        if (!surface) {
            ADD_FAILURE() << "Échec de la création de la surface de test";
            return "";
        }

        // Sauvegarde en PNG
        if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".png") {
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
    EXPECT_NO_THROW(manager.setCursorPath(testImage));

    // Test avec un chemin invalide - ne lance pas d'exception, affiche juste une erreur
    EXPECT_NO_THROW(manager.setCursorPath("fichier_inexistant.png"));
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

    // Test avec des valeurs invalides (doivent être clampées)
    EXPECT_NO_THROW(manager.setAnimationSpeed(-10));
    EXPECT_NO_THROW(manager.setAnimationSpeed(1000));
}
// Test de changement de curseur
TEST_F(CursorManagerTest, ChangeCursor) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());

    // Créer un curseur de test
    std::string testImage = createTestImage("test_cursor.png");
    ASSERT_FALSE(testImage.empty());

    // Changer de curseur
    manager.setCursorPath(testImage);

    // Vérifier que le rendu fonctionne toujours
    manager.render();
    SUCCEED();
}
// Test d'activation/désactivation du curseur
TEST_F(CursorManagerTest, ToggleCursor) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());

    // Désactiver le curseur
    manager.setEnabled(false);
    EXPECT_FALSE(manager.isEnabled());

    // Réactiver le curseur
    manager.setEnabled(true);
    EXPECT_TRUE(manager.isEnabled());

    // Vérification que le rendu fonctionne toujours
    manager.render();
    SUCCEED();
}
// Test de rendu du curseur
TEST_F(CursorManagerTest, Render) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());

    // Test de rendu du curseur
    manager.render();

    // Vérifier que le rendu fonctionne sans erreur
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Chargement d'un curseur de test
    std::string testImage = createTestImage("test_render.png");
    ASSERT_FALSE(testImage.empty());

    // Rendu avec un curseur chargé
    manager.setCursorPath(testImage);
    manager.render();

    // Vérification que le rendu s'est bien effectué (pas de crash)
    SUCCEED();
}
// Test de performance du rendu
TEST_F(CursorManagerTest, Performance) {
    CursorManager manager;
    ASSERT_TRUE(manager.initialize());

    // Mesurer le temps de rendu
    auto start = std::chrono::high_resolution_clock::now();

    // Effectuer plusieurs rendus
    for (int i = 0; i < 100; ++i) {
        manager.render();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Vérifier que le rendu est suffisamment rapide (moins de 100ms pour 100 rendus)
    EXPECT_LT(duration, 100);

    std::cout << "Temps de rendu pour 100 frames: " << duration << "ms" << std::endl;
}
