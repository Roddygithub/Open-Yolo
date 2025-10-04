#include <gtest/gtest.h>
#include "cursormanager/CursorManager.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <fstream>

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
    }
    
    static void TearDownTestSuite() {
        IMG_Quit();
        SDL_Quit();
    }
    
    void SetUp() override {
        // Création d'un répertoire temporaire pour les tests
        testDir = std::filesystem::temp_directory_path() / "open-yolo-theme-test";
        std::filesystem::create_directories(testDir);
        
        // Initialisation du gestionnaire de curseurs
        manager = &CursorManager::getInstance();
    }
    
    void TearDown() override {
        // Nettoyage du répertoire de test
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
            if (testImage.empty()) return false;
            
            std::filesystem::copy_file(testImage, cursorPath, 
                std::filesystem::copy_options::overwrite_existing);
                
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error creating test theme: " << e.what() << std::endl;
            return false;
        }
    }
    
    CursorManager* manager;
    std::filesystem::path testDir;
};

// Test d'exportation de profil
TEST_F(ThemeManagerTest, ExportProfile) {
    // Créer un profil de test
    CursorProfile testProfile;
    testProfile.name = "test_export";
    testProfile.isSystem = false;
    
    // Ajouter un curseur de test
    std::string cursorPath = createTestImage("test_cursor.png");
    ASSERT_FALSE(cursorPath.empty());
    
    CustomCursor cursor;
    cursor.name = "test_cursor";
    cursor.path = cursorPath;
    cursor.size = 32;
    cursor.hotspotX = 0;
    cursor.hotspotY = 0;
    testProfile.cursors[CursorType::Pointer] = cursor;
    
    // Ajouter le profil
    ASSERT_TRUE(manager->createProfile(testProfile.name, testProfile));
    
    // Exporter le profil
    std::string exportPath = (testDir / "exported_profile.yolo").string();
    bool exportResult = manager->exportProfile(testProfile.name, exportPath);
    
    // Vérifier que l'exportation a réussi
    ASSERT_TRUE(exportResult);
    ASSERT_TRUE(std::filesystem::exists(exportPath));
    
    // Vérifier que le fichier n'est pas vide
    ASSERT_GT(std::filesystem::file_size(exportPath), 0);
}

// Test d'importation de profil
TEST_F(ThemeManagerTest, ImportProfile) {
    // Créer un profil de test
    CursorProfile testProfile;
    testProfile.name = "test_import";
    testProfile.isSystem = false;
    
    // Ajouter un curseur de test
    std::string cursorPath = createTestImage("test_cursor_import.png");
    ASSERT_FALSE(cursorPath.empty());
    
    CustomCursor cursor;
    cursor.name = "test_cursor_import";
    cursor.path = cursorPath;
    cursor.size = 32;
    cursor.hotspotX = 0;
    cursor.hotspotY = 0;
    testProfile.cursors[CursorType::Pointer] = cursor;
    
    // Ajouter le profil
    ASSERT_TRUE(manager->createProfile(testProfile.name, testProfile));
    
    // Exporter le profil pour l'importer ensuite
    std::string exportPath = (testDir / "import_test_profile.yolo").string();
    ASSERT_TRUE(manager->exportProfile(testProfile.name, exportPath));
    
    // Supprimer le profil original
    ASSERT_TRUE(manager->deleteProfile(testProfile.name));
    
    // Importer le profil
    bool importResult = manager->importProfile(exportPath, false);
    
    // Vérifier que l'importation a réussi
    ASSERT_TRUE(importResult);
    
    // Vérifier que le profil a bien été importé
    auto profiles = manager->listProfiles();
    bool found = false;
    for (const auto& profile : profiles) {
        if (profile == testProfile.name) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

// Test de chargement de thème
TEST_F(ThemeManagerTest, LoadTheme) {
    // Créer une structure de thème de test
    std::string themeDir = (testDir / "test_theme").string();
    ASSERT_TRUE(createTestTheme("Test Theme", themeDir));
    
    // Charger le thème
    bool loadResult = manager->loadTheme(themeDir);
    
    // Vérifier que le chargement a réussi
    ASSERT_TRUE(loadResult);
    
    // Vérifier que le thème est bien chargé
    auto themes = manager->listAvailableThemes();
    bool found = false;
    for (const auto& theme : themes) {
        if (theme == "Test Theme") {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

// Test d'application d'un thème à un profil
TEST_F(ThemeManagerTest, ApplyThemeToProfile) {
    // Créer un thème de test
    std::string themeDir = (testDir / "test_theme_apply").string();
    ASSERT_TRUE(createTestTheme("Test Apply Theme", themeDir));
    
    // Charger le thème
    ASSERT_TRUE(manager->loadTheme(themeDir));
    
    // Créer un profil de test
    std::string profileName = "test_apply_theme";
    CursorProfile testProfile;
    testProfile.name = profileName;
    testProfile.isSystem = false;
    
    // Ajouter le profil
    ASSERT_TRUE(manager->createProfile(profileName, testProfile));
    
    // Appliquer le thème au profil
    bool applyResult = manager->applyThemeToProfile("Test Apply Theme", profileName);
    
    // Vérifier que l'application du thème a réussi
    ASSERT_TRUE(applyResult);
    
    // Charger le profil pour vérifier que le thème a été appliqué
    ASSERT_TRUE(manager->loadProfile(profileName));
    
    // Vérifier que le profil utilise bien le thème
    // (cette partie dépend de l'implémentation de getCurrentProfile ou d'une méthode similaire)
    SUCCEED();
}

// Test d'installation d'un thème à partir d'une archive
TEST_F(ThemeManagerTest, InstallTheme) {
    // Créer un thème de test
    std::string themeDir = (testDir / "test_theme_install").string();
    ASSERT_TRUE(createTestTheme("Test Install Theme", themeDir));
    
    // Créer une archive du thème
    std::string archivePath = (testDir / "test_theme_install.tar.gz").string();
    std::string command = "tar -czf " + archivePath + " -C " + testDir.string() + " test_theme_install";
    int result = std::system(command.c_str());
    ASSERT_EQ(result, 0) << "Failed to create theme archive";
    
    // Installer le thème
    bool installResult = manager->installTheme(archivePath);
    
    // Vérifier que l'installation a réussi
    ASSERT_TRUE(installResult);
    
    // Vérifier que le thème est disponible
    auto themes = manager->listAvailableThemes();
    bool found = false;
    for (const auto& theme : themes) {
        if (theme == "Test Install Theme") {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}
