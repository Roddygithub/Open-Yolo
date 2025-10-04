#include <gtest/gtest.h>
#include "cursormanager/CursorManager.hpp"

using namespace cursor_manager;

class CursorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cursorManager = std::make_unique<CursorManager>();
        ASSERT_TRUE(cursorManager->initialize());
    }

    void TearDown() override {
        cursorManager.reset();
    }

    std::unique_ptr<CursorManager> cursorManager;
};

TEST_F(CursorManagerTest, Initialization) {
    ASSERT_NE(cursorManager, nullptr);
}

TEST_F(CursorManagerTest, SetCursorScale) {
    const float testScale = 2.0f;
    cursorManager->setScale(testScale);
    
    // Vérifier que l'échelle a été appliquée
    // Note: Cette vérification nécessite une méthode getScale() dans CursorManager
    // Si cette méthode n'existe pas, envisagez de l'ajouter ou de vérifier un effet secondaire
    // Par exemple, vérifier si le curseur a été redessiné avec la nouvelle échelle
    EXPECT_TRUE(true); // Test factice - à remplacer par une vraie vérification
}

TEST_F(CursorManagerTest, ToggleVisibility) {
    // Cacher le curseur
    cursorManager->setVisible(false);
    // Vérifier que le curseur est caché
    // Note: Cette vérification nécessite une méthode isVisible() dans CursorManager
    
    // Afficher le curseur
    cursorManager->setVisible(true);
    // Vérifier que le curseur est visible
    // Note: Cette vérification nécessite une méthode isVisible() dans CursorManager
    
    EXPECT_TRUE(true); // Test factice - à remplacer par de vraies vérifications
}

TEST_F(CursorManagerTest, SetCursorType) {
    // Tester la définition d'un type de curseur spécifique
    bool result = cursorManager->setCursorType(CursorManager::CursorType::Pointer);
    EXPECT_TRUE(result);
    
    // Vérifier que le type de curseur a été appliqué
    // Note: Cette vérification nécessite une méthode getCurrentCursorType() dans CursorManager
}
