#pragma once

#include <functional>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <gtkmm.h>
#include <gdkmm/event.h>
#include <gdkmm/display.h>
#include <gdkmm/device.h>
#include <gdkmm/seat.h>
#include <gdkmm/window.h>

// Forward declarations
namespace cursor_manager {
    class CursorManager;
}

// Déclaration anticipée de MainWindow
class MainWindow;

class InputManager {
public:
    using KeyCallback = std::function<void()>;
    
    // Structure pour stocker les informations d'un raccourci
    struct Shortcut {
        std::string accelerator;
        guint accelKey;
        Gdk::ModifierType mods;
        KeyCallback callback;
        
        Shortcut() = default;
        Shortcut(const std::string& accel, guint key, Gdk::ModifierType modifiers, const KeyCallback& cb)
            : accelerator(accel), accelKey(key), mods(modifiers), callback(cb) {}
    };
    
    InputManager();
    ~InputManager();
    
    /**
     * Initialise le gestionnaire d'entrée
     * @return true si l'initialisation a réussi, false sinon
     */
    bool initialize();
    
    /**
     * Configure l'intégration avec GTK et la fenêtre principale
     * @param window Pointeur vers la fenêtre principale
     * @param cursorManager Pointeur partagé vers le gestionnaire de curseur
     */
    void setupGTKIntegration(MainWindow* window, std::shared_ptr<cursor_manager::CursorManager> cursorManager);
    
    /**
     * Enregistre un nouveau raccourci clavier
     * @param name Nom unique identifiant le raccourci
     * @param accelerator Chaîne de raccourci au format GTK (ex: "<Control>F10")
     * @param callback Fonction à appeler lorsque le raccourci est activé
     * @return true si le raccourci a été enregistré avec succès, false en cas de conflit
     */
    bool registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback);
    
    /**
     * Supprime un raccourci enregistré
     * @param name Nom du raccourci à supprimer
     */
    void removeShortcut(const std::string& name);
    
    /**
     * Analyse une chaîne d'accélérateur GTK et extrait le code de touche et les modificateurs
     * @param accel Chaîne d'accélérateur (ex: "<Control>F10")
     * @param key Code de touche de sortie
     * @param mods Modificateurs de sortie (Ctrl, Shift, etc.)
     * @return true si l'analyse a réussi, false sinon
     */
    bool parseAccelerator(const std::string& accel, guint& key, Gdk::ModifierType& mods) const;
    
private:
    // Méthodes privées
    bool onKeyPressed(GdkEventKey* event);
    void initializeInputDevices();
    bool checkForDeviceChanges(); // Vérification périodique des périphériques
    void onDeviceRemoved(const Glib::RefPtr<Gdk::Device>& device); // Gestion de la suppression d'un périphérique
    
    // Données membres
    Glib::RefPtr<Gdk::Display> display_;
    Glib::RefPtr<Gdk::DeviceManager> deviceManager_;
    
    // Gestion des périphériques
    std::set<Glib::RefPtr<Gdk::Device>> keyboardDevices_;
    std::mutex devicesMutex_;
    
    // Gestion des raccourcis
    std::map<std::string, Shortcut> shortcuts_;
    mutable std::mutex shortcutsMutex_;
    
    // Références aux autres composants
    MainWindow* mainWindow_ = nullptr;
    std::shared_ptr<cursor_manager::CursorManager> cursorManager_;
    
    // Constantes
    static constexpr const char* LOG_DOMAIN = "InputManager";
};
