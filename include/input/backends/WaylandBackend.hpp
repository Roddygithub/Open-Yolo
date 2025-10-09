#ifndef WAYLAND_BACKEND_HPP
#define WAYLAND_BACKEND_HPP

#include <gdk/gdkwayland.h>
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>
#include <gtkmm.h>
#include <sigc++/connection.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "../InputBackend.hpp"

// Spécialisation de std::hash pour Glib::ustring
namespace std {
template <>
struct hash<Glib::ustring> {
    std::size_t operator()(const Glib::ustring& s) const noexcept {
// Utilisation de la méthode raw() si disponible, sinon utiliser std::string
#if GLIBMM_MAJOR_VERSION > 2 || (GLIBMM_MAJOR_VERSION == 2 && GLIBMM_MINOR_VERSION >= 68)
        return std::hash<std::string>{}(s.raw());
#else
        return std::hash<std::string>{}(s);
#endif
    }
};
}  // namespace std

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
// Déclaration anticipée de la classe InputManager
class InputManager;

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

    using StateChangeCallback = std::function<void(SessionState oldState, SessionState newState)>;

    WaylandBackend(InputManager* manager);
    ~WaylandBackend() override;

    // Implémentation de l'interface InputBackend
    bool isInitialized() const override { return m_initialized; }
    bool initialize() override;
    bool isAvailable() const override;
    void registerShortcut(const std::string& name, const std::string& accelerator,
                          KeyCallback callback) override;
    void unregisterShortcut(const std::string& name) override;
    bool handleKeyEvent(guint keyval, GdkModifierType mods) override;

    /**
     * @brief Gestionnaire d'événements de touche pressée (obsolète, utiliser handleKeyEvent).
     * @param event L'événement de touche.
     * @return true si l'événement a été traité, false sinon.
     * @deprecated Utiliser handleKeyEvent à la place.
     */
    bool onKeyPressed(GdkEventKey* event) override;

    // Méthodes spécifiques à Wayland
    void setWindow(Gtk::Window* window);

    /**
     * @brief Arrête le backend et libère les ressources.
     * @return true si l'arrêt a réussi, false sinon.
     */
    bool shutdown() override;

    void setMinLogLevel(LogLevel level);
    void addStateChangeCallback(StateChangeCallback&& callback);

private:
    void bindExistingShortcuts(const Glib::ustring& session_handle);

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

    // Callback pour les erreurs
    using ErrorCallback = std::function<void(const std::string& message)>;

    // Méthodes de gestion des erreurs
    void handleError(const std::string& message, const Glib::Error* e = nullptr);
    void handleError(const std::string& message, const std::exception* e);
    void addErrorCallback(ErrorCallback&& callback);

    // Gestion des raccourcis
    void onBindShortcutsResponse(Glib::RefPtr<Gio::AsyncResult>& result);

    // Vérification de la validité du proxy
    bool isProxyValid() const;

    /**
     * @brief Planifie une mise à jour des raccourcis
     * @details Cette méthode est utilisée pour regrouper plusieurs mises à jour de raccourcis
     * en une seule opération, en évitant les appels redondants au service D-Bus.
     */
    void scheduleUpdate();

    // Membres
    InputManager* m_manager;
    Glib::RefPtr<Gio::DBus::Connection> m_connection;
    Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_sessionProxy;
    Glib::ustring m_sessionHandle;

    // État
    struct ShortcutData {
        std::string accelerator;
        guint keyval = 0;
        Gdk::ModifierType mods = Gdk::ModifierType(0);
        std::function<void()> callback;

        // Constructeur par défaut
        ShortcutData() = default;

        // Constructeur avec accélérateur et callback
        ShortcutData(const std::string& accel, const std::function<void()>& cb)
            : accelerator(accel), callback(cb) {
            // Convertir l'accélérateur en keyval et mods
            Gdk::ModifierType mods = Gdk::ModifierType(0);
            guint keyval = 0;

            // La méthode parse ne retourne pas de bool, on suppose qu'elle modifie les références
            // passées en paramètre
            Gtk::AccelGroup::parse(accel, keyval, mods);
            this->keyval = keyval;
            this->mods = mods;
        }

        // Opérateur d'assignation depuis un callback
        ShortcutData& operator=(std::function<void()> cb) {
            callback = cb;
            return *this;
        }
    };

    LogLevel m_minLogLevel = LogLevel::Info;
    std::vector<StateChangeCallback> m_stateChangeCallbacks;
    std::vector<std::function<void(const std::string&)>> m_errorCallbacks;

    // Connexions et états
    sigc::connection m_reconnectConnection;
    std::unordered_map<std::string, ShortcutData> m_shortcuts;
    std::unordered_map<std::string, std::string> m_pendingShortcuts;
    sigc::connection m_updateTimer;
    sigc::connection m_reconnectTimer;
    int m_retryCount = 0;
    SessionState m_sessionState = SessionState::Disconnected;
    bool m_initialized;
};

}  // namespace input
#endif  // WAYLAND_BACKEND_HPP