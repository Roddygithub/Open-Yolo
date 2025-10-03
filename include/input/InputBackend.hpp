#ifndef INPUT_BACKEND_HPP
#define INPUT_BACKEND_HPP

#include <gtkmm/window.h>
#include <functional>
#include <string>
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
     * @brief Initialise le backend.
     * @return true si l'initialisation a réussi, false sinon.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Définit la fenêtre principale de l'application.
     * @param window Pointeur vers la fenêtre GTK.
     */
    virtual void setWindow(Gtk::Window* window) = 0;

    /**
     * @brief Enregistre un raccourci.
     * @param name Nom unique du raccourci.
     * @param accelerator Chaîne de l'accélérateur (ex: "<Control>F10").
     * @param callback Fonction à appeler.
     * @return true si l'enregistrement a réussi.
     */
    virtual bool registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) = 0;

    /**
     * @brief Supprime un raccourci enregistré.
     * @param name Nom unique du raccourci à supprimer.
     * @return true si la suppression a réussi.
     */
    virtual bool unregisterShortcut(const std::string& name) = 0;

    virtual bool onKeyPressed(GdkEventKey* event) = 0;

};

} // namespace input
#endif // INPUT_BACKEND_HPP