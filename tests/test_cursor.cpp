#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "cursormanager/CursorManager.hpp"

namespace fs = std::filesystem;
using namespace cursor_manager;

class CursorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cursorManager = std::make_unique<CursorManager>();
        cursorManager->initialize();

        // Créer un fichier de curseur de test
        testDir = fs::temp_directory_path() / "open-yolo-test";
        fs::create_directories(testDir);
        testCursorPath = testDir / "test_cursor.png";
        std::ofstream testFile(testCursorPath);
        testFile << "PNG test file";
        testFile.close();
    }

    void TearDown() override {
        cursorManager.reset();
        fs::remove_all(testDir);
    }

    std::unique_ptr<CursorManager> cursorManager;
    fs::path testDir;
    fs::path testCursorPath;
};

TEST_F(CursorManagerTest, Initialization) { ASSERT_NE(cursorManager, nullptr); }

TEST_F(CursorManagerTest, SetCursorScale) {
    const float testScale = 2.0f;
    cursorManager->setScale(testScale);

    // Vérifier que l'échelle a été appliquée
    EXPECT_FLOAT_EQ(cursorManager->getScale(), testScale);
}

TEST_F(CursorManagerTest, ToggleEnabled) {
    // Vérifier que le curseur est activé par défaut
    EXPECT_TRUE(cursorManager->isEnabled());

    // Désactiver le curseur
    cursorManager->setEnabled(false);
    EXPECT_FALSE(cursorManager->isEnabled());

    // Réactiver le curseur
    cursorManager->setEnabled(true);
    EXPECT_TRUE(cursorManager->isEnabled());
}

TEST_F(CursorManagerTest, SetCursorPath) {
    // Tester la définition du chemin du curseur
    cursorManager->setCursorPath(testCursorPath.string());
    EXPECT_EQ(cursorManager->getCurrentCursorPath(), testCursorPath.string());

    // Vérifier que le gestionnaire est toujours actif après le changement
    EXPECT_TRUE(cursorManager->isEnabled());
}
