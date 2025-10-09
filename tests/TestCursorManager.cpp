#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>

#include "cursormanager/CursorManager.hpp"

namespace fs = std::filesystem;
using namespace cursor_manager;

class CursorManagerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Créer un répertoire temporaire pour les tests
        testDir = fs::temp_directory_path() / "open-yolo-test";
        fs::create_directories(testDir);

        // Créer un fichier de curseur de test
        testCursorPath = testDir / "test_cursor.png";
        std::ofstream testFile(testCursorPath);
        testFile << "PNG test file";
        testFile.close();

        // Initialiser le gestionnaire de curseurs
        manager = std::make_unique<CursorManager>();
        manager->initialize();
    }

    static void TearDownTestSuite() {
        // Nettoyer les fichiers temporaires
        fs::remove_all(testDir);
    }

    void SetUp() override {
        // Réinitialiser l'état avant chaque test
        if (manager) {
            manager->setEnabled(true);
            manager->setCursorPath(testCursorPath.string());
        }
    }

    static std::unique_ptr<CursorManager> manager;
    static fs::path testDir;
    static fs::path testCursorPath;
};

std::unique_ptr<CursorManager> CursorManagerTest::manager = nullptr;
fs::path CursorManagerTest::testDir;
fs::path CursorManagerTest::testCursorPath;

TEST_F(CursorManagerTest, Initialization) {
    ASSERT_NE(manager, nullptr);
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(CursorManagerTest, SetCursorPath) {
    // Tester la définition d'un chemin de curseur personnalisé
    const std::string testPath = "/test/path/to/cursor.png";
    manager->setCursorPath(testPath);
    EXPECT_EQ(manager->getCurrentCursorPath(), testPath);
}

TEST_F(CursorManagerTest, ScaleManagement) {
    // Tester la définition de l'échelle
    const float testScale = 2.0f;
    manager->setScale(testScale);
    EXPECT_FLOAT_EQ(manager->getScale(), testScale);
}

TEST_F(CursorManagerTest, AnimationSpeedManagement) {
    // Tester la définition de la vitesse d'animation (FPS)
    const int testFps = 30;
    manager->setAnimationSpeed(testFps);
    EXPECT_EQ(manager->getAnimationSpeed(), testFps);
}

TEST_F(CursorManagerTest, EnableDisable) {
    // Tester l'activation/désactivation
    manager->setEnabled(false);
    EXPECT_FALSE(manager->isEnabled());

    manager->setEnabled(true);
    EXPECT_TRUE(manager->isEnabled());
}

TEST_F(CursorManagerTest, Visibility) {
    // Tester la visibilité
    // Note: La visibilité est liée à l'activation dans cette implémentation
    manager->setEnabled(true);
    EXPECT_TRUE(manager->isEnabled());

    manager->setEnabled(false);
    EXPECT_FALSE(manager->isEnabled());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
