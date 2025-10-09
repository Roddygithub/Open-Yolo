#include <iostream>
#include <thread>
#include <chrono>
#include <input/CursorManager.hpp>

using namespace input;

int main() {
    // Obtenir l'instance du gestionnaire de curseurs
    auto& cursorManager = CursorManager::getInstance();
    
    // Initialiser le gestionnaire
    if (!cursorManager.initialize()) {
        std::cerr << "Failed to initialize cursor manager" << std::endl;
        return 1;
    }

    std::cout << "Cursor manager initialized successfully" << std::endl;

    try {
        // Créer un profil personnalisé
        CursorManager::CursorProfile customProfile{"Custom", {}, false};
        
        // Ajouter des curseurs personnalisés (remplacer les chemins par des vrais fichiers)
        customProfile.cursors[CursorManager::CursorType::Pointer] = 
            {"Hand", "resources/cursors/hand_cursor.png", 0, 0, 32};
        
        // Créer le profil
        if (!cursorManager.createProfile("MyProfile", customProfile)) {
            std::cerr << "Failed to create profile" << std::endl;
            return 1;
        }

        // Charger le profil
        if (!cursorManager.loadProfile("MyProfile")) {
            std::cerr << "Failed to load profile" << std::endl;
            return 1;
        }

        std::cout << "Custom cursor profile loaded. Press Enter to continue..." << std::endl;
        std::cin.get();

        // Faire clignoter le curseur
        for (int i = 0; i < 5; ++i) {
            cursorManager.setCursorVisible(false);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            cursorManager.setCursorVisible(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // Utiliser un curseur système
        cursorManager.useSystemCursors(true);
        std::cout << "Using system cursors. Press Enter to exit..." << std::endl;
        std::cin.get();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
