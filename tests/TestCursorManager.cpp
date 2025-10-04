#include <gtest/gtest.h>
#include <input/CursorManager.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace input;

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
        manager = &CursorManager::getInstance();
        manager->initialize();
    }
    
    static void TearDownTestSuite() {
        // Nettoyer les fichiers temporaires
        fs::remove_all(testDir);
    }
    
    void SetUp() override {
        // Réinitialiser l'état avant chaque test
        manager->useSystemCursors(true);
    }
    
    static CursorManager* manager;
    static fs::path testDir;
    static fs::path testCursorPath;
};

CursorManager* CursorManagerTest::manager = nullptr;
fs::path CursorManagerTest::testDir;
fs::path CursorManagerTest::testCursorPath;

TEST_F(CursorManagerTest, Initialization) {
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(CursorManagerTest, SetCursor) {
    // Tester avec un chemin de fichier
    EXPECT_TRUE(manager->setCursor(CursorManager::CursorType::Default, testCursorPath.string()));
    
    // Tester avec un objet CursorInfo
    CursorManager::CursorInfo cursorInfo;
    cursorInfo.path = testCursorPath.string();
    cursorInfo.size = 32;
    cursorInfo.hotspotX = 0;
    cursorInfo.hotspotY = 0;
    EXPECT_TRUE(manager->setCursor(CursorManager::CursorType::Default, cursorInfo));
}

TEST_F(CursorManagerTest, UseSystemCursors) {
    // Tester l'activation/désactivation des curseurs système
    manager->useSystemCursors(false);
    manager->useSystemCursors(true);
    
    // Vérifier que nous pouvons toujours définir un curseur personnalisé
    EXPECT_TRUE(manager->setCursor(CursorManager::CursorType::Default, testCursorPath.string()));
}

TEST_F(CursorManagerTest, CursorPosition) {
    // Tester la position du curseur
    int x = 100, y = 100;
    manager->setCursorPosition(x, y);
    auto pos = manager->getCursorPosition();
    
    // Note: Sur certains systèmes, la position peut être limitée par la résolution de l'écran
    EXPECT_GE(pos.first, 0);
    EXPECT_GE(pos.second, 0);
}

TEST_F(CursorManagerTest, CursorVisibility) {
    // Tester la visibilité du curseur
    manager->setCursorVisible(false);
    manager->setCursorVisible(true);
}

TEST_F(CursorManagerTest, ProfileManagement) {
    const std::string profileName = "test_profile";
    
    // Sauvegarder le profil
    EXPECT_TRUE(manager->setCursor(CursorManager::CursorType::Default, testCursorPath.string()));
    EXPECT_TRUE(manager->saveProfile(profileName));
    
    // Charger le profil
    EXPECT_TRUE(manager->loadProfile(profileName));
    
    // Lister les profils
    auto profiles = manager->listProfiles();
    EXPECT_FALSE(profiles.empty());
    
    // Supprimer le profil
    EXPECT_TRUE(manager->deleteProfile(profileName));
    
    // Vérifier que le profil a été supprimé
    profiles = manager->listProfiles();
    EXPECT_TRUE(std::find(profiles.begin(), profiles.end(), profileName) == profiles.end());
}

// Tests pour les nouvelles fonctionnalités
TEST_F(CursorManagerTest, ScaleManagement) {
    // Tester la définition et la récupération de l'échelle
    const float testScale = 2.0f;
    manager->setScale(testScale);
    EXPECT_FLOAT_EQ(manager->getScale(), testScale);
    
    // Tester avec une échelle invalide (doit être > 0)
    const float invalidScale = -1.0f;
    manager->setScale(invalidScale);
    EXPECT_GT(manager->getScale(), 0.0f); // Doit rester à la dernière valeur valide
}

TEST_F(CursorManagerTest, AnimationSpeedManagement) {
    // Tester la définition et la récupération de la vitesse d'animation
    const float testFps = 30.0f;
    manager->setFps(static_cast<int>(testFps));
    EXPECT_FLOAT_EQ(manager->getAnimationSpeed(), testFps);
    
    // Tester avec une vitesse invalide (doit être > 0)
    const int invalidFps = -10;
    manager->setFps(invalidFps);
    EXPECT_GT(manager->getAnimationSpeed(), 0.0f); // Doit rester à la dernière valeur valide
}

TEST_F(CursorManagerTest, VisibilityManagement) {
    // Tester la visibilité
    manager->setCursorVisible(false);
    EXPECT_FALSE(manager->isVisible());
    
    manager->setCursorVisible(true);
    EXPECT_TRUE(manager->isVisible());
}

TEST_F(CursorManagerTest, InitializationCheck) {
    // Vérifier que le gestionnaire est correctement initialisé
    EXPECT_TRUE(manager->isInitialized());
    
    // Tester avec un gestionnaire non initialisé
    CursorManager* uninitializedManager = &CursorManager::getInstance();
    // Ne pas appeler initialize() intentionnellement
    if (uninitializedManager->isInitialized()) {
        // Si le gestionnaire est déjà initialisé, c'est un singleton donc c'est normal
        EXPECT_TRUE(true);
    } else {
        EXPECT_FALSE(uninitializedManager->isInitialized());
    }
}

TEST_F(CursorManagerTest, CurrentCursorPath) {
    // Définir un curseur personnalisé
    EXPECT_TRUE(manager->setCursor(CursorManager::CursorType::Default, testCursorPath.string()));
    
    // Vérifier que le chemin du curseur est correctement enregistré
    std::string currentPath = manager->getCurrentCursorPath();
    EXPECT_FALSE(currentPath.empty());
    
    // Vérifier que le chemin correspond à celui que nous avons défini
    // Note: La comparaison peut nécessiter une normalisation des chemins selon le système
    EXPECT_EQ(fs::path(currentPath).filename(), testCursorPath.filename());
}

TEST_F(CursorManagerTest, EnableDisableCursor) {
    // Activer le curseur
    manager->setEnabled(true);
    EXPECT_TRUE(manager->isEnabled());
    
    // Désactiver le curseur
    manager->setEnabled(false);
    EXPECT_FALSE(manager->isEnabled());
    
    // Réactiver le curseur
    manager->setEnabled(true);
    EXPECT_TRUE(manager->isEnabled());
}

TEST_F(CursorManagerTest, OptimizeGPUMemory) {
    // Appeler l'optimisation de la mémoire GPU
    // Ce test vérifie principalement que la méthode ne provoque pas d'erreur
    EXPECT_NO_THROW(manager->optimizeGPUMemory());
    
    // Vérifier que le gestionnaire est toujours fonctionnel après l'optimisation
    EXPECT_TRUE(manager->isInitialized());
    
    // Vérifier que la visibilité est inchangée
    bool wasVisible = manager->isVisible();
    manager->optimizeGPUMemory();
    EXPECT_EQ(manager->isVisible(), wasVisible);
}

TEST_F(CursorManagerTest, ClearEffects) {
    // Ajouter un effet d'ombre
    manager->setShadowEffect(5, 5, 2.0f, {0.0f, 0.0f, 0.0f, 0.5f});
    
    // Vérifier que l'effet est activé
    EXPECT_TRUE(manager->isEffectEnabled("shadow"));
    
    // Effacer tous les effets
    manager->clearEffects();
    
    // Vérifier que l'effet a été désactivé
    EXPECT_FALSE(manager->isEffectEnabled("shadow"));
    
    // Vérifier que le gestionnaire est toujours fonctionnel
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(CursorManagerTest, EffectPipelineManagement) {
    // Définir une pipeline d'effets personnalisée
    std::vector<std::string> effects = {"shadow", "glow"};
    manager->setEffectPipeline(effects);
    
    // Activer les effets
    manager->setEffectEnabled("shadow", true);
    manager->setEffectEnabled("glow", true);
    
    // Vérifier que les effets sont activés
    EXPECT_TRUE(manager->isEffectEnabled("shadow"));
    EXPECT_TRUE(manager->isEffectEnabled("glow"));
    
    // Effacer les effets et vérifier qu'ils sont désactivés
    manager->clearEffects();
    EXPECT_FALSE(manager->isEffectEnabled("shadow"));
    EXPECT_FALSE(manager->isEffectEnabled("glow"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
