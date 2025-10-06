#ifndef INPUT_BACKEND_HPP
#define INPUT_BACKEND_HPP

// Standard C++
#include <functional>
#include <string>

// GTKmm
#include <gtkmm/window.h>
#include <gdk/gdk.h>

namespace input {

/**
 * @class InputBackend
 * @brief Interface abstraite pour la gestion des entrées spécifiques au serveur d'affichage.
 *
 * Cette classe définit une interface commune pour les backends X11 et Wayland,
 * permettant d'abstraire la logique de capture des raccourcis clavier.
 */
class InputBackend {
public:
    using KeyCallback = std::function<void()>;

    virtual ~InputBackend() = default;

    /**
     * @brief Vérifie si le backend est initialisé.
     * @return true si le backend est initialisé, false sinon.
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Initialise le backend.
     * @return true si l'initialisation a réussi, false sinon.
     */
    virtual bool initialize() = 0;

    /**
     * @return true si le backend est disponible, false sinon.
     */
    virtual bool isAvailable() const = 0;

    /**
     * @brief Enregistre un raccourci.
     * @param name Nom unique du raccourci.
     * @param accelerator Chaîne de l'accélérateur (ex: "<Control>F10").
     * @param callback Fonction à appeler.
     */
    virtual void registerShortcut(const std::string& name, const std::string& accelerator, KeyCallback callback) = 0;
    
    /**
     * @brief Désenregistre un raccourci.
     * @param name Nom du raccourci à désenregistrer.
     */
    virtual void unregisterShortcut(const std::string& name) = 0;
    
    /**
     * @brief Gère un événement clavier.
     * @param keyval Code de la touche.
     * @param mods Modificateurs (Ctrl, Alt, etc.).
     * @return true si l'événement a été traité, false sinon.
     */
    virtual bool handleKeyEvent(guint keyval, GdkModifierType mods) = 0;
    
    /**
     * @brief Gestionnaire d'événements de touche pressée (obsolète, utiliser handleKeyEvent).
     * @param event L'événement de touche.
     * @return true si l'événement a été traité, false sinon.
     * @deprecated Utiliser handleKeyEvent à la place.
     */
    virtual bool onKeyPressed(GdkEventKey* event) = 0;

    /**
     * @brief Arrête le backend et libère les ressources.
     * @return true si l'arrêt a réussi, false sinon.
     */
    virtual bool shutdown() = 0;
};

} // namespace input
#endif // INPUT_BACKEND_HPP