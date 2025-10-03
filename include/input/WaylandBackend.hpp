#ifndef WAYLAND_BACKEND_HPP
#define WAYLAND_BACKEND_HPP

#include "InputBackend.hpp"
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <stdexcept>
#include <giomm/dbusproxy.h>
#include <giomm/dbusconnection.h>
#include <gtkmm/window.h>
#include <sigc++/connection.h>

// Forward declare std::filesystem::path
namespace std {
    namespace filesystem {
        class path;
    }
}
namespace fs = std::filesystem;

namespace input {

class WaylandBackend : public InputBackend {
public:
    WaylandBackend();
    ~WaylandBackend() override;

    bool initialize() override;
    void setWindow(Gtk::Window* window) override;
    bool registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) override;
    bool unregisterShortcut(const std::string& name) override;
    bool onKeyPressed(GdkEventKey* event) override;

    enum class SessionState {
        Disconnected,
        Connecting,
        Connected,
        Reconnecting
    };

    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    using StateChangeCallback = std::function<void(SessionState, SessionState)>;
    using ErrorCallback = std::function<void(const std::string& error_message)>;

    void addStateChangeCallback(StateChangeCallback&& callback);
    void addErrorCallback(ErrorCallback&& callback);
    void setMinLogLevel(LogLevel level);

protected:
    virtual void recreateSession();
    virtual void closeSession();

private:
    struct Shortcut {
        std::string accelerator;
        KeyCallback callback;
    };

    void scheduleUpdate();

    /**
     * @brief Callback appelé lors de la création d'un proxy D-Bus.
     * @param result Résultat de l'opération asynchrone.
     * @throws std::runtime_error Si la création du proxy échoue.
     */
    void onProxyCreated(Glib::RefPtr<Gio::AsyncResult>& result);
    
    void createPortalSession();
    void onSessionCreated(Glib::RefPtr<Gio::AsyncResult>& result);
    void onSessionShortcutActivated(
        const Glib::ustring& sender_name,
        const Glib::ustring& signal_name,
        const Glib::VariantContainerBase& parameters);
    
    void setSessionState(SessionState newState);
    void scheduleReconnect();
    void ensureConnected();
    std::string convertToPortalShortcut(const std::string& gtkAccel);

    void handleError(const std::string& message, const std::exception* e = nullptr);
    bool isProxyValid() const;

    Gtk::Window* m_window;
    bool m_initialized;
    
    Glib::RefPtr<Gio::DBus::Connection> m_dbusConnection;
    Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_sessionProxy;
    std::string m_sessionHandle;

    std::map<std::string, Shortcut> m_shortcuts;
    
    SessionState m_sessionState;
    int m_retryCount;
    LogLevel m_minLogLevel = LogLevel::Info;
    std::vector<StateChangeCallback> m_stateChangeCallbacks;
    std::vector<ErrorCallback> m_errorCallbacks;
    sigc::connection m_updateTimer;
};

} // namespace input

#endif // WAYLAND_BACKEND_HPP
