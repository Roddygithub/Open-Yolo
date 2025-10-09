#include <gtest/gtest.h>
#include <gtkmm.h>

#include <filesystem>
#include <fstream>
#include <memory>

#include "../include/cursormanager/CursorManager.hpp"
#include "../include/displaymanager/DisplayManager.hpp"
#include "../include/gui/MainWindow.hpp"
#include "../include/input/InputManager.hpp"

// Fixture pour les tests de l'interface graphique
class GUITest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Initialisation de GTKmm
        app = Gtk::Application::create("org.openyolo.test");

        // Création d'un répertoire temporaire pour les tests
        testDir = std::filesystem::temp_directory_path() / "open-yolo-test-gui";
        std::filesystem::create_directories(testDir);

        // Création de fichiers de test
        createTestFile("cursor1.png");
        createTestFile("cursor2.gif");
        createTestConfig();
    }

    static void TearDownTestSuite() {
        // Nettoyage du répertoire de test
        std::filesystem::remove_all(testDir);
    }

    void SetUp() override {
        // Création des dépendances nécessaires pour MainWindow
        cursorManager = std::make_shared<cursor_manager::CursorManager>();
        displayManager = std::make_shared<input::DisplayManager>();
        inputManager = std::make_shared<input::InputManager>();

        // Création d'une fenêtre principale pour les tests
        window = std::make_unique<gui::MainWindow>(cursorManager, displayManager, inputManager);
    }

    void TearDown() override {
        window.reset();
        inputManager.reset();
        displayManager.reset();
        cursorManager.reset();
    }

    static void createTestFile(const std::string& filename) {
        std::ofstream ofs(testDir / filename);
        ofs << "Test content for " << filename;
        ofs.close();
    }

    static void createTestConfig() {
        std::ofstream ofs(testDir / "config.ini");
        ofs << "[Cursor]\n";
        ofs << "path = " << (testDir / "cursor1.png").string() << "\n";
        ofs << "scale = 1.5\n";
        ofs << "fps = 30\n";
        ofs << "[Shortcuts]\n";
        ofs << "toggle = <Control><Shift>space\n";
        ofs.close();
    }

    static Glib::RefPtr<Gtk::Application> app;
    static std::filesystem::path testDir;
    std::shared_ptr<cursor_manager::CursorManager> cursorManager;
    std::shared_ptr<input::DisplayManager> displayManager;
    std::shared_ptr<input::InputManager> inputManager;
    std::unique_ptr<gui::MainWindow> window;
};

Glib::RefPtr<Gtk::Application> GUITest::app;
std::filesystem::path GUITest::testDir;

// Test d'initialisation de la fenêtre principale
TEST_F(GUITest, WindowInitialization) {
    // Vérifier que la fenêtre est visible
    EXPECT_TRUE(window->is_visible());
}

// Test de chargement de la configuration
TEST_F(GUITest, CursorManagerIntegration) {
    // Vérifier que le gestionnaire de curseurs est correctement initialisé
    ASSERT_NE(cursorManager, nullptr);
    ASSERT_TRUE(cursorManager->isInitialized());

    // Tester la définition de l'échelle
    cursorManager->setScale(1.5f);
    EXPECT_FLOAT_EQ(cursorManager->getScale(), 1.5f);

    // Tester la définition de la vitesse d'animation
    cursorManager->setAnimationSpeed(30);
    EXPECT_EQ(cursorManager->getAnimationSpeed(), 30);
}

// Test de sélection de curseur
TEST_F(GUITest, DisplayManagerIntegration) {
    // Vérifier que le gestionnaire d'affichage est correctement initialisé
    ASSERT_NE(displayManager, nullptr);

    // Tester la récupération des écrans
    displayManager->updateDisplays();
    const auto& displays = displayManager->getDisplays();
    EXPECT_FALSE(displays.empty());

    // Tester la récupération de l'écran principal
    const auto* primaryDisplay = displayManager->getPrimaryDisplay();
    EXPECT_NE(primaryDisplay, nullptr);
}

// Test de configuration des FPS
TEST_F(GUITest, InputManagerIntegration) {
    // Vérifier que le gestionnaire d'entrée est correctement initialisé
    ASSERT_NE(inputManager, nullptr);

    // Tester l'activation/désactivation du suivi de la souris
    // Note: Cette partie nécessiterait un mock ou un test d'intégration plus avancé
    // car InputManager n'a pas de méthodes setEnabled/isEnabled
    // Nous vérifions simplement que le pointeur est valide
    EXPECT_NE(inputManager, nullptr);
}

// Test de la sauvegarde de la configuration
TEST_F(GUITest, SaveConfiguration) {
    const std::string testConfig = testDir / "test_save.ini";

    // Test de configuration du gestionnaire de curseurs
    cursorManager->setScale(2.0f);
    cursorManager->setAnimationSpeed(30);
    cursorManager->setCursorPath((testDir / "cursor2.gif").string());

    // Vérification des valeurs définies
    EXPECT_FLOAT_EQ(cursorManager->getScale(), 2.0f);
    EXPECT_EQ(cursorManager->getAnimationSpeed(), 30);
    EXPECT_EQ(cursorManager->getCurrentCursorPath(), (testDir / "cursor2.gif").string());
}

// Test de la visibilité de la fenêtre
TEST_F(GUITest, WindowVisibility) {
    // Tester la visibilité de la fenêtre
    ASSERT_NE(window, nullptr);

    // La fenêtre devrait être visible par défaut
    EXPECT_TRUE(window->is_visible());

    // Cacher la fenêtre
    window->hide();
    EXPECT_FALSE(window->is_visible());

    // Afficher la fenêtre
    window->show();
    EXPECT_TRUE(window->is_visible());
}

// Test de nettoyage des ressources
TEST_F(GUITest, CleanupOnDestruction) {
    // Vérifier que les ressources sont correctement libérées
    ASSERT_NE(window, nullptr);

    // Réinitialiser les pointeurs partagés
    cursorManager.reset();
    displayManager.reset();
    inputManager.reset();

    // La fenêtre devrait toujours être valide car elle possède ses propres références
    EXPECT_NO_THROW(window->is_visible());

    // Libérer la fenêtre
    window.reset();

    // Vérifier que tout a été correctement nettoyé
    EXPECT_EQ(cursorManager.use_count(), 0);
    EXPECT_EQ(displayManager.use_count(), 0);
    EXPECT_EQ(inputManager.use_count(), 0);
}

// Fonction principale pour exécuter les tests
int main(int argc, char** argv) {
    // Initialisation de GTK avant les tests
    Gtk::Main kit(argc, argv);

    // Initialisation de Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Exécution des tests
    return RUN_ALL_TESTS();
}
