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
#include <fstream>
#include <thread>

using namespace cursor_manager;

// Fixture pour les tests de gestion des thèmes
class ThemeManagerTest : public ::testing::Test {
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

        // Vérification de la version d'OpenGL
        const GLubyte* version = glGetString(GL_VERSION);
        if (!version) {
            FAIL() << "Impossible de récupérer la version d'OpenGL";
        }
        std::cout << "OpenGL version: " << version << std::endl;
    }

    static void TearDownTestSuite() {
        IMG_Quit();
        SDL_Quit();
    }

    void SetUp() override {
        // Création d'un répertoire temporaire pour les tests
        testDir = std::filesystem::temp_directory_path() / "open-yolo-theme-test";
        std::filesystem::create_directories(testDir);

        // Création d'une fenêtre SDL pour le contexte OpenGL
        window =
            SDL_CreateWindow("Test ThemeManager", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

        // Initialisation du gestionnaire de curseurs
        manager = std::make_unique<CursorManager>();
        if (!manager->initialize()) {
            FAIL() << "Échec de l'initialisation du gestionnaire de curseurs";
        }
    }

    void TearDown() override {
        // Nettoyage du gestionnaire de curseurs
        manager.reset();

        // Nettoyage du contexte OpenGL
        if (glContext) {
            SDL_GL_DeleteContext(glContext);
        }

        // Nettoyage de la fenêtre
        if (window) {
            SDL_DestroyWindow(window);
        }

        // Nettoyage du répertoire de test
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

        SDL_FreeSurface(surface);
        return path;
    }

    // Crée un thème de test dans le répertoire spécifié
    bool createTestTheme(const std::string& themeName, const std::string& themeDir) {
        try {
            std::string cursorsDir = themeDir + "/cursors";
            std::filesystem::create_directories(cursorsDir);

            // Créer un fichier index.theme
            std::ofstream indexFile(themeDir + "/index.theme");
            indexFile << "[Icon Theme]\n";
            indexFile << "Name=" << themeName << "\n";
            indexFile << "Comment=Test theme for unit tests\n";
            indexFile << "Example=cursor\n";
            indexFile << "Cursors=default\n";
            indexFile.close();

            // Créer un curseur de test
            std::string cursorPath = cursorsDir + "/default.png";
            std::string testImage = createTestImage("default_cursor.png");
            if (testImage.empty())
                return false;

            std::filesystem::copy_file(testImage, cursorPath,
                                       std::filesystem::copy_options::overwrite_existing);

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error creating test theme: " << e.what() << std::endl;
            return false;
        }
    }

    std::unique_ptr<CursorManager> manager;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    std::filesystem::path testDir;
};

// Test de fonctionnalité de base
TEST_F(ThemeManagerTest, BasicFunctionality) {
    // Vérifier que le gestionnaire est bien initialisé
    ASSERT_TRUE(manager->isInitialized());

    // Tester la méthode setCursor avec un chemin valide
    std::string testImage = createTestImage("test_cursor.png");
    ASSERT_FALSE(testImage.empty()) << "Échec de la création de l'image de test";

    // Tester le chargement d'un curseur
    manager->setCursorPath(testImage);

    // Tester le rendu
    manager->render();

    // Tester l'activation/désactivation
    manager->setEnabled(false);
    EXPECT_FALSE(manager->isEnabled());

    manager->setEnabled(true);
    EXPECT_TRUE(manager->isEnabled());

    // Tester le changement d'échelle
    manager->setScale(1.5f);
    EXPECT_FLOAT_EQ(manager->getScale(), 1.5f);

    // Tester le changement de vitesse d'animation
    manager->setAnimationSpeed(30);
    EXPECT_EQ(manager->getAnimationSpeed(), 30);
}

// Test de performance du rendu
TEST_F(ThemeManagerTest, PerformanceTest) {
    // Vérifier que le gestionnaire est bien initialisé
    ASSERT_TRUE(manager->isInitialized());

    // Charger un curseur de test
    std::string testImage = createTestImage("test_cursor.png");
    ASSERT_FALSE(testImage.empty()) << "Échec de la création de l'image de test";
    manager->setCursorPath(testImage);

    // Mesurer le temps de rendu
    const int numFrames = 100;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numFrames; ++i) {
        manager->render();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Afficher les résultats
    std::cout << "Temps de rendu pour " << numFrames << " frames: " << duration << "ms"
              << std::endl;
    std::cout << "FPS moyen: " << (numFrames * 1000.0 / duration) << std::endl;

    // Vérifier que le rendu est raisonnablement rapide (moins de 10ms par frame en moyenne)
    EXPECT_LT(duration, numFrames * 10);
}

// Test de gestion des erreurs
TEST_F(ThemeManagerTest, ErrorHandling) {
    // Tester avec un chemin de curseur invalide
    EXPECT_NO_THROW(manager->setCursorPath("fichier_inexistant.png"));

    // Tester avec une échelle invalide
    EXPECT_NO_THROW(manager->setScale(-1.0f));
    EXPECT_GE(manager->getScale(), 0.1f);  // Doit être clampé à la valeur minimale

    // Tester avec une vitesse d'animation invalide
    EXPECT_NO_THROW(manager->setAnimationSpeed(-10));
    EXPECT_GT(manager->getAnimationSpeed(), 0);  // Doit être supérieur à 0

    // Tester avec une vitesse d'animation très élevée
    EXPECT_NO_THROW(manager->setAnimationSpeed(1000));
    EXPECT_LT(manager->getAnimationSpeed(),
              1000);  // Doit être inférieur à une valeur maximale raisonnable

    // Tester le rendu même en cas d'erreur précédente
    EXPECT_NO_THROW(manager->render());
}

// Test de configuration de l'échelle
TEST_F(ThemeManagerTest, ScaleConfiguration) {
    // Tester différentes valeurs d'échelle
    const float testScales[] = {0.5f, 1.0f, 1.5f, 2.0f, 2.5f};

    for (float scale : testScales) {
        manager->setScale(scale);
        EXPECT_FLOAT_EQ(manager->getScale(), scale);

        // Vérifier que le rendu fonctionne avec cette échelle
        EXPECT_NO_THROW(manager->render());
    }

    // Tester avec des valeurs limites
    manager->setScale(0.1f);  // Valeur minimale
    EXPECT_GE(manager->getScale(), 0.1f);

    manager->setScale(5.0f);  // Valeur maximale raisonnable
    EXPECT_LE(manager->getScale(), 5.0f);

    // Tester avec des valeurs invalides
    manager->setScale(-1.0f);
    EXPECT_GE(manager->getScale(), 0.1f);  // Doit être clampé à la valeur minimale

    manager->setScale(0.0f);
    EXPECT_GE(manager->getScale(), 0.1f);  // Doit être clampé à la valeur minimale

    manager->setAnimationSpeed(1000.0f);
    EXPECT_LT(manager->getAnimationSpeed(),
              1000);  // Doit être inférieur à une valeur maximale raisonnable
}
