#include <gtest/gtest.h>
#include <gtkmm.h>
#include "../src/gui/MainWindow.hpp"
#include <filesystem>
#include <fstream>

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
        // Création d'une fenêtre principale pour les tests
        window = std::make_unique<MainWindow>();
    }
    
    void TearDown() override {
        window.reset();
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
    std::unique_ptr<MainWindow> window;
};

Glib::RefPtr<Gtk::Application> GUITest::app;
std::filesystem::path GUITest::testDir;

// Test d'initialisation de la fenêtre principale
TEST_F(GUITest, WindowInitialization) {
    ASSERT_NE(window, nullptr);
    EXPECT_TRUE(window->is_visible());
}

// Test de chargement de la configuration
TEST_F(GUITest, LoadConfiguration) {
    // Vérification des valeurs par défaut
    EXPECT_FLOAT_EQ(window->getCursorScale(), 1.0f);
    EXPECT_EQ(window->getFPS(), 60);
    
    // Chargement de la configuration de test
    window->loadConfig((testDir / "config.ini").string());
    
    // Vérification des valeurs chargées
    EXPECT_FLOAT_EQ(window->getCursorScale(), 1.5f);
    EXPECT_EQ(window->getFPS(), 30);
    EXPECT_EQ(window->getCursorPath(), (testDir / "cursor1.png").string());
}

// Test de sélection de curseur
TEST_F(GUITest, CursorSelection) {
    // Simulation de la sélection d'un fichier
    window->onCursorSelected((testDir / "cursor2.gif").string());
    
    // Vérification que le chemin du curseur a été mis à jour
    EXPECT_EQ(window->getCursorPath(), (testDir / "cursor2.gif").string());
    
    // Vérification que le type de curseur est correctement détecté
    EXPECT_TRUE(window->isAnimatedCursor());
}

// Test de configuration des FPS
TEST_F(GUITest, FPSConfiguration) {
    // Modification du FPS
    const int testFPS = 24;
    window->setFPS(testFPS);
    
    // Vérification que le FPS a été mis à jour
    EXPECT_EQ(window->getFPS(), testFPS);
    
    // Vérification que la configuration est sauvegardée
    window->saveConfig((testDir / "test_save.ini").string());
    
    // Vérification du fichier de configuration sauvegardé
    std::ifstream ifs(testDir / "test_save.ini");
    std::string content((std::istreambuf_iterator<char>(ifs)), 
                       (std::istreambuf_iterator<char>()));
    EXPECT_NE(content.find("fps = 24"), std::string::npos);
}

// Test des raccourcis clavier
TEST_F(GUITest, KeyboardShortcuts) {
    // Configuration d'un raccourci de test
    const std::string shortcut = "<Control><Shift>space";
    window->setToggleShortcut(shortcut);
    
    // Vérification que le raccourci a été configuré
    EXPECT_EQ(window->getToggleShortcut(), shortcut);
    
    // Simulation de l'appui sur le raccourci
    // Note: Ce test est simplifié, en réalité il faudrait simuler les événements clavier
    bool toggled = false;
    window->signal_toggle_visibility().connect([&toggled]() { toggled = true; });
    
    // Simulation de l'événement (à implémenter selon votre système d'événements)
    // window->simulateKeyPress(shortcut);
    
    // Vérification que l'événement a été émis
    // EXPECT_TRUE(toggled);
}

// Test de la sauvegarde de la configuration
TEST_F(GUITest, SaveConfiguration) {
    const std::string testConfig = testDir / "test_config.ini";
    
    // Modification de la configuration
    window->setCursorScale(2.0f);
    window->setFPS(30);
    window->setCursorPath((testDir / "cursor2.gif").string());
    
    // Sauvegarde de la configuration
    window->saveConfig(testConfig);
    
    // Vérification que le fichier a été créé
    EXPECT_TRUE(std::filesystem::exists(testConfig));
    
    // Vérification du contenu du fichier
    std::ifstream ifs(testConfig);
    std::string content((std::istreambuf_iterator<char>(ifs)), 
                       (std::istreambuf_iterator<char>()));
    
    EXPECT_NE(content.find("scale = 2"), std::string::npos);
    EXPECT_NE(content.find("fps = 30"), std::string::npos);
    EXPECT_NE(content.find("cursor2.gif"), std::string::npos);
}

// Fonction principale pour exécuter les tests
int main(int argc, char **argv) {
    // Initialisation de GTK avant les tests
    Gtk::Main kit(argc, argv);
    
    // Initialisation de Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Exécution des tests
    return RUN_ALL_TESTS();
}
