#ifndef WAYLAND_BACKEND_HPP
#define WAYLAND_BACKEND_HPP

#include <giomm/dbusproxy.h>
#include <glibmm/refptr.h>
#include <gtkmm.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "../InputBackend.hpp"

namespace input {

class InputManager;

class WaylandBackend : public InputBackend {
public:
    explicit WaylandBackend(InputManager* manager);

    // Implémentation de InputBackend
    bool initialize() override;
    void registerShortcut(const std::string& name, const std::string& accelerator,
                          std::function<void()> callback) override;
    void unregisterShortcut(const std::string& name) override;

private:
    // États de la session
    enum class SessionState {
        DISCONNECTED,  // Non connecté
        CONNECTING,    // Connexion en cours
        CONNECTED,     // Connecté et opérationnel
        ERROR          // Erreur de connexion
    };

private:
    void onSessionShortcutActivated(const Glib::ustring& sender_name,
                                    const Glib::ustring& signal_name,
                                    const Glib::VariantContainerBase& parameters);

    void onBindShortcutsResponse(Glib::RefPtr<Gio::AsyncResult> result);
    void onSessionCreated(Glib::RefPtr<Gio::AsyncResult> result);
    // Méthodes utilitaires
    void recreateSession();
    void bindExistingShortcuts(const Glib::ustring& session_handle);
    void closeSession();
    void scheduleReconnect();
    void setSessionState(SessionState state);
    std::string convertToPortalShortcut(const std::string& accelerator) const;

    // Callbacks de changement d'état
    std::vector<std::function<void(SessionState, SessionState)>> m_stateChangeCallbacks;

    InputManager* m_manager;
    Glib::RefPtr<Gio::DBus::Connection> m_connection;
    Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
    std::string m_sessionHandle;
    std::map<std::string, std::function<void()>> m_shortcuts;
    sigc::connection m_updateTimer;
    sigc::connection m_reconnectTimer;
    SessionState m_sessionState;
    int m_retryCount;
    bool m_initialized;
};

}  // namespace input

#endif  // WAYLAND_BACKEND_HPP
