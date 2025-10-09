#ifndef WAYLAND_BACKEND_HPP
#define WAYLAND_BACKEND_HPP

#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>

#include <memory>
#include <unordered_map>

#include "../InputBackend.hpp"

namespace input {

/**
 * @class WaylandBackend
 * @brief Implémentation du backend d'entrée pour le serveur d'affichage Wayland.
 *
 * Cette classe est responsable de la gestion des entrées (raccourcis clavier) dans un environnement
 * Wayland en utilisant le portail D-Bus XDG pour les raccourcis globaux.
 *
 * @details Elle gère un cycle de vie d'états (Disconnected, Connecting, Connected, Reconnecting)
 * pour assurer une connexion robuste au service D-Bus. La journalisation est effectuée dans un
 * format structuré (similaire à JSON) pour faciliter le débogage.
 * @example
 * auto backend = std::make_unique<WaylandBackend>();
 * backend->setMinLogLevel(WaylandBackend::LogLevel::Debug);
 * backend->addStateChangeCallback([](auto oldState, auto newState) {
 *     std::cout << "State changed: " << to_string(oldState) << " -> " << to_string(newState) <<
 * std::endl;
 * });
 */
class WaylandBackend : public InputBackend {
public:
    /// Niveaux de journalisation pour le backend.
    enum class LogLevel { Debug, Info, Warning, Error };

    /// État de la session D-Bus.
    /// Les transitions d'état valides sont :
    /// - Disconnected -> Connecting
    /// - Connecting   -> Connected | Reconnecting | Disconnected
    /// - Connected    -> Disconnected
    /// - Reconnecting -> Connecting | Disconnected
    enum class SessionState {
        Disconnected,  ///< Pas de session active ou de connexion.
        Connecting,    ///< Connexion au proxy ou création de la session en cours.
        Connected,     ///< Connecté et opérationnel.
        Reconnecting   ///< En attente d'une nouvelle tentative de connexion.
    };

    /// Callback pour les changements d'état de la session.
    using StateChangeCallback = std::function<void(SessionState oldState, SessionState newState)>;

    WaylandBackend();
    ~WaylandBackend() override;

    bool initialize() override;
    void setWindow(Gtk::Window* window) override;
    /**
     * @brief Enregistre un raccourci.
     * @param name Nom unique du raccourci.
     * @param accelerator Chaîne de l'accélérateur (ex: "<Control>F10").
     * @param callback Fonction à appeler.
     * @return true si l'enregistrement a réussi.
     * @example
     * backend->registerShortcut("screenshot", "<Control><Alt>S", []() { ... });
     */
    bool registerShortcut(const std::string& name, const std::string& accelerator,
                          const KeyCallback& callback) override;
    /**
     * @brief Supprime un raccourci enregistré.
     * @param name Nom du raccourci à supprimer.
     * @return true si le raccourci a été trouvé et que sa suppression a été initiée, false sinon.
     * @example
     * // Exemple d'utilisation :
     * backend->unregisterShortcut("screenshot");
     * // La session sera automatiquement recréée avec les raccourcis restants.
     */
    bool unregisterShortcut(const std::string& name) override;
    bool onKeyPressed(GdkEventKey* event) override;

    void setMinLogLevel(LogLevel level);
    void addStateChangeCallback(StateChangeCallback&& callback);

private:
    // Gestion D-Bus
    /**
     * @brief Callback appelé après la tentative de création du proxy D-Bus.
     * @param result Le résultat de l'opération asynchrone.
     * @example
     * // La connexion au signal est faite ici :
     * m_proxy->signal_signal().connect(sigc::mem_fun(*this,
     * &WaylandBackend::onSessionShortcutActivated));
     */
    void onProxyCreated(Glib::RefPtr<Gio::AsyncResult>& result);
    void createPortalSession();
    void onSessionCreated(Glib::RefPtr<Gio::AsyncResult>& result);
    void onSessionShortcutActivated(const Glib::ustring& sender_name,
                                    const Glib::ustring& signal_name,
                                    const Glib::VariantContainerBase& parameters);
    /**
     * @brief Tente de se reconnecter au service D-Bus après un échec.
     * @details Cette méthode planifie une nouvelle tentative de connexion après un délai.
     * Le nombre de tentatives est limité.
     */
    void scheduleReconnect();
    /**
     * @brief Ferme la session D-Bus actuelle.
     * @note Cette méthode est idempotente et peut être appelée plusieurs fois sans effet
     * secondaire.
     */
    void closeSession();
    /**
     * @brief Recrée la session D-Bus pour refléter la liste actuelle des raccourcis.
     * @details Ferme la session existante et en crée une nouvelle. Si la liste des raccourcis est
     * vide, la session est simplement fermée sans en créer une nouvelle. En cas d'échec, une
     * reconnexion est planifiée.
     * @throws std::runtime_error Si la recréation de la session échoue (bien que gérée en interne).
     */
    void recreateSession();
    /**
     * @brief Convertit un accélérateur au format GTK (ex: "<Control>F10") au format du portail
     * D-Bus ("Control+F10").
     * @param gtkAccel La chaîne de l'accélérateur au format GTK.
     * @return La chaîne de l'accélérateur convertie.
     */
    std::string convertToPortalShortcut(const std::string& gtkAccel);
    void setSessionState(SessionState newState);
    void ensureConnected();

    // Membres
    Gtk::Window* m_window;
    Glib::RefPtr<Gio::DBus::Connection> m_dbusConnection;
    Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_sessionProxy;
    Glib::ustring m_sessionHandle;

    // État
    struct ShortcutData {
        std::string accelerator;
        KeyCallback callback;
    };

    LogLevel m_minLogLevel = LogLevel::Info;
    std::vector<StateChangeCallback> m_stateChangeCallbacks;
    int m_retryCount = 0;
    SessionState m_sessionState = SessionState::Disconnected;
    bool m_initialized;
    std::unordered_map<Glib::ustring, ShortcutData> m_shortcuts;
};

}  // namespace input
#endif  // WAYLAND_BACKEND_HPP