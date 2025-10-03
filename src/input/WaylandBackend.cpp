#include "../../include/input/WaylandBackend.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>

// Includes spécifiques à Wayland via GDK
#include <gdk/gdkwayland.h>
#include <gdkmm/window.h>
#include <giomm/dbusproxy.h>
#include <giomm/dbusconnection.h>

namespace input {

// ... (Namespace with constants and logging helpers)

// --- Implementation ---

WaylandBackend::WaylandBackend()
    : m_window(nullptr),
      m_initialized(false),
      m_sessionState(SessionState::Disconnected),
      m_retryCount(0) {}

WaylandBackend::~WaylandBackend() {
    if (m_updateTimer.connected()) {
        m_updateTimer.disconnect();
    }
    closeSession();
}

void WaylandBackend::handleError(const std::string& message, const std::exception* e) {
    std::string full_message = message;
    if (e) {
        full_message += ": " + std::string(e->what());
    }
    LOG_ERROR(full_message);
    for (const auto& cb : m_errorCallbacks) {
        cb(full_message);
    }
}

bool WaylandBackend::isProxyValid() const {
    return m_proxy && m_proxy->get_name_owner();
}

bool WaylandBackend::initialize() {
    m_retryCount = 0;
    auto gdkDisplay = Gdk::Display::get_default();
    if (!gdkDisplay || !GDK_IS_WAYLAND_DISPLAY(gdkDisplay->gobj())) {
        handleError("Le backend Wayland ne peut être initialisé que sur un affichage Wayland.");
        return false;
    }
    setSessionState(SessionState::Connecting);
    try {
        m_dbusConnection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    } catch (const Glib::Error& e) {
        handleError("Erreur de connexion au bus D-Bus de session", &e);
        setSessionState(SessionState::Disconnected);
        return false;
    }
    Gio::DBus::Proxy::create(
        m_dbusConnection,
        DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE,
        [this](Glib::RefPtr<Gio::AsyncResult>& result) { onProxyCreated(result); }
    );
    m_initialized = true;
    LOG_INFO("Initialisation du backend Wayland (via D-Bus Portal) en cours...");
    return true;
}

void WaylandBackend::onProxyCreated(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        m_proxy = Gio::DBus::Proxy::create_finish(result);
        if (!isProxyValid()) {
            throw std::runtime_error("Le service de portail XDG n'est pas disponible. Vérifiez que xdg-desktop-portal est installé et en cours d'exécution.");
        }
        LOG_INFO("Proxy D-Bus pour GlobalShortcuts créé. Fourni par: " + m_proxy->get_name_owner());
        createPortalSession();
    } catch (const std::runtime_error& e) {
        handleError("Erreur lors de la création du proxy D-Bus", &e);
        setSessionState(SessionState::Reconnecting);
        scheduleReconnect();
    }
}

void WaylandBackend::createPortalSession() {
    if (!isProxyValid()) {
        handleError("Proxy D-Bus non valide ou service non disponible pour créer une session.");
        scheduleReconnect();
        return;
    }
    try {
        std::map<Glib::ustring, Glib::VariantBase> options;
        options["handle_token"] = Glib::Variant<Glib::ustring>::create("open_yolo_session");
        auto params = Glib::VariantContainerBase::create_tuple({ Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>::create(options) });
        m_proxy->call(
            "CreateSession",
            [this](Glib::RefPtr<Gio::AsyncResult>& result) { this->onSessionCreated(result); },
            params
        );
    } catch (const Glib::Error& e) {
        handleError("Erreur lors de l'appel à CreateSession", &e);
        setSessionState(SessionState::Reconnecting);
        scheduleReconnect();
    }
}

void WaylandBackend::closeSession() {
    if (m_sessionHandle.empty()) return;

    if (!isProxyValid()) {
        LOG_WARNING("Proxy D-Bus non valide, impossible de fermer proprement la session.");
    } else {
        try {
            m_proxy->call("CloseSession", Glib::VariantContainerBase::create_tuple({ Glib::Variant<Glib::ustring>::create(m_sessionHandle) }));
        } catch (const std::exception& e) {
            handleError("Erreur (ignorée) lors de la demande de fermeture de la session D-Bus", &e);
        }
    }
    m_sessionHandle.clear();
    m_sessionProxy.reset();
    setSessionState(SessionState::Disconnected);
}

void WaylandBackend::scheduleUpdate() {
    if (m_updateTimer.connected()) {
        m_updateTimer.disconnect();
    }
    m_updateTimer = Glib::signal_timeout().connect([this]() {
        if (m_sessionState == SessionState::Connected) {
            try {
                if (m_shortcuts.empty()) {
                    closeSession();
                } else {
                    recreateSession();
                }
            } catch (const std::exception& e) {
                handleError("Échec de la mise à jour des raccourcis", &e);
            }
        }
        m_updateTimer = {};
        return false;
    }, DEBOUNCE_DELAY_MS);
}

bool WaylandBackend::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    if (m_sessionState != SessionState::Connected) {
        LOG_WARNING("Cannot register shortcut: Not connected to session");
        return false;
    }

    if (name.empty()) {
        handleError("Shortcut name cannot be empty");
        return false;
    }

    if (!callback) {
        handleError("Callback for shortcut '" + name + "' cannot be null");
        return false;
    }

    try {
        // Convert accelerator to portal format
        std::string portalShortcut = convertToPortalShortcut(accelerator);
        if (portalShortcut.empty()) {
            LOG_ERROR("Failed to convert shortcut: " + accelerator);
            return false;
        }

        // Check if shortcut already exists
        if (m_shortcuts.find(name) != m_shortcuts.end()) {
            LOG_WARNING("Shortcut already registered: " + name);
            return false;
        }

        // Register shortcut with the portal
        auto params = Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.Desktop"),
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.GlobalShortcuts"),
            Glib::Variant<Glib::ustring>::create("BindShortcuts"),
            Glib::wrap(g_variant_new_parsed("(@a(sa{sv})a{sv})",
                g_variant_new_parsed(
                    "[('%s', {'shortcut': <%s>})]",
                    name.c_str(),
                    portalShortcut.c_str()
                ),
                g_variant_new_parsed("{'handle_token': <%s>}", "openyolo")
            ))
        });

        m_proxy->call_sync("Call", params);
        
        // Store the callback
        m_shortcuts[name] = { accelerator, callback };
        
        LOG_DEBUG("Registered shortcut: " + name + " -> " + accelerator);
        return true;
    } catch (const Glib::Error& e) {
        handleError("Failed to register shortcut: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        handleError("Unexpected error while registering shortcut", &e);
        return false;
    }
}

bool WaylandBackend::unregisterShortcut(const std::string& name) {
    if (m_sessionState != SessionState::Connected) {
        LOG_WARNING("Cannot unregister shortcut: Not connected to session");
        return false;
    }

    if (name.empty()) {
        handleError("Shortcut name cannot be empty");
        return false;
    }

    try {
        // Check if shortcut exists
        auto it = m_shortcuts.find(name);
        if (it == m_shortcuts.end()) {
            LOG_WARNING("Shortcut not found: " + name);
            return false;
        }

        // Unregister shortcut from the portal
        auto params = Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.Desktop"),
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.GlobalShortcuts"),
            Glib::Variant<Glib::ustring>::create("UnbindShortcuts"),
            Glib::wrap(g_variant_new_parsed("(@as)",
                g_variant_new_parsed("['%s']", name.c_str())
            ))
        });

        m_proxy->call_sync("Call", params);
        
        // Remove the callback
        m_shortcuts.erase(it);
        
        LOG_DEBUG("Unregistered shortcut: " + name);
        return true;
    } catch (const Glib::Error& e) {
        handleError("Failed to unregister shortcut: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        handleError("Unexpected error while unregistering shortcut", &e);
        return false;
    }
}

// ... (Le reste des fonctions reste globalement inchangé mais pourrait bénéficier de l'utilisation de handleError)

std::string WaylandBackend::convertToPortalShortcut(const std::string& gtkAccel) {
    // Convert GTK accelerator (e.g., "<Control>q") to portal format (e.g., "<Control>q")
    // This is a basic implementation and might need adjustments
    std::string result = gtkAccel;
    
    // Convert key names to lowercase for portal
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    // Replace key names if needed
    // Example: "Control" -> "Ctrl"
    size_t pos = 0;
    while ((pos = result.find("control", pos)) != std::string::npos) {
        result.replace(pos, 7, "Ctrl");
        pos += 4;
    }
    
    // Clean up any extra spaces
    result.erase(std::unique(result.begin(), result.end(),
        [](char a, char b) { return a == ' ' && b == ' '; }), result.end());
    
    return result;
}

#pragma region "Unchanged Methods"

void WaylandBackend::setWindow(Gtk::Window* window) { m_window = window; }

bool WaylandBackend::onKeyPressed(GdkEventKey* event) { return false; }

void WaylandBackend::setMinLogLevel(LogLevel level) { m_minLogLevel = level; }

void WaylandBackend::addStateChangeCallback(StateChangeCallback&& callback) { m_stateChangeCallbacks.push_back(std::move(callback)); }

void WaylandBackend::addErrorCallback(ErrorCallback&& callback) { m_errorCallbacks.push_back(std::move(callback)); }

void WaylandBackend::setSessionState(SessionState newState) {
    static const std::map<SessionState, std::set<SessionState>> validTransitions = {
        {SessionState::Disconnected, {SessionState::Connecting}},
        {SessionState::Connecting,   {SessionState::Connected, SessionState::Reconnecting, SessionState::Disconnected}},
        {SessionState::Connected,    {SessionState::Disconnected}},
        {SessionState::Reconnecting, {SessionState::Connecting, SessionState::Disconnected}}
    };
    if (m_sessionState == newState) return;
    if (validTransitions.count(m_sessionState) == 0 || validTransitions.at(m_sessionState).count(newState) == 0) {
        handleError("Transition d'état invalide: " + to_string(m_sessionState) + " -> " + to_string(newState));
        return;
    }
    SessionState oldState = m_sessionState;
    m_sessionState = newState;
    for (const auto& callback : m_stateChangeCallbacks) {
        try {
            callback(oldState, newState);
        } catch (const std::exception& e) {
            handleError("Erreur dans le callback de changement d'état", &e);
        }
    }
}

void WaylandBackend::onSessionCreated(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        auto res = m_proxy->call_finish(result);
        Glib::Variant<Glib::ustring> handle_variant;
        res.get_child(handle_variant);
        m_sessionHandle = handle_variant.get();
        setSessionState(SessionState::Connected);

        using ShortcutArray = std::vector<std::tuple<Glib::ustring, Glib::ustring, std::map<Glib::ustring, Glib::VariantBase>>>;
        ShortcutArray shortcuts_to_bind;
        for (const auto& pair : m_shortcuts) {
            shortcuts_to_bind.emplace_back(pair.first, convertToPortalShortcut(pair.second.accelerator), std::map<Glib::ustring, Glib::VariantBase>{});
        }
        auto shortcuts_variant = Glib::Variant<ShortcutArray>::create(shortcuts_to_bind);
        auto params = Glib::VariantContainerBase::create_tuple({ Glib::Variant<Glib::ustring>::create(m_sessionHandle), shortcuts_variant });

        m_proxy->call("BindShortcuts", params,
            [this](Glib::RefPtr<Gio::AsyncResult>& bind_result) {
                 try {
                     m_proxy->call_finish(bind_result);
                 } catch (const Glib::Error& e) {
                     handleError("Erreur lors de la liaison des raccourcis", &e);
                 }
            }
        );
        m_proxy->signal_signal().connect(sigc::mem_fun(*this, &WaylandBackend::onSessionShortcutActivated));
    } catch (const Glib::Error& e) {
        handleError("Erreur lors de la création de la session D-Bus", &e);
        scheduleReconnect();
    }
}

bool WaylandBackend::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    if (m_sessionState != SessionState::Connected) {
        LOG_WARNING("Cannot register shortcut: Not connected to session");
        return false;
    }

    try {
        auto it = m_shortcuts.find(name);
        if (it == m_shortcuts.end()) {
            LOG_WARNING("Shortcut not found: " + name);
            return false;
        }

        // Supprimer le raccourci du portail
        auto params = Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.Desktop"),
            Glib::Variant<Glib::ustring>::create("org.freedesktop.portal.GlobalShortcuts"),
            Glib::Variant<Glib::ustring>::create("UnbindShortcuts"),
            Glib::wrap(g_variant_new_parsed("(@as)",
                g_variant_new_parsed("['%s']", name.c_str())
            ))
        });

        m_proxy->call_sync("Call", params);
        
        // Supprimer le callback
        m_shortcuts.erase(it);
        
        LOG_DEBUG("Unregistered shortcut: " + name);
        return true;
    } catch (const Glib::Error& e) {
        handleError("Failed to unregister shortcut: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        handleError("Unexpected error while unregistering shortcut", &e);
        return false;
    if (signal_name != "GlobalShortcutFired" || parameters.get_n_children() < 2) return;
    try {
{{ ... }}
        parameters.get_children(params);
        Glib::ustring session_handle = params.get<0>();
        Glib::ustring shortcut_id = params.get<1>();
        if (session_handle != m_sessionHandle) return;
        auto it = m_shortcuts.find(shortcut_id);
        if (it != m_shortcuts.end()) {
            it->second.callback();
        }
    } catch (const Glib::Error& e) {
        handleError("Erreur lors du traitement du signal GlobalShortcutFired", &e);
    }
}

void WaylandBackend::scheduleReconnect() {
    if (m_retryCount < MAX_RETRIES) {
        m_retryCount++;
        Glib::signal_timeout().connect_seconds([this]() { initialize(); return false; }, RETRY_DELAY_SEC);
    }
}

void WaylandBackend::recreateSession() {
    try {
        closeSession();
        if (!m_shortcuts.empty()) {
            createPortalSession();
        }
    } catch (const std::exception& e) {
        handleError("Échec de la recréation de la session", &e);
        scheduleReconnect();
    }
}

std::string WaylandBackend::convertToPortalShortcut(const std::string& gtkAccel) {
    std::string portalAccel = gtkAccel;
    portalAccel.erase(std::remove(portalAccel.begin(), portalAccel.end(), '<'), portalAccel.end());
    std::replace(portalAccel.begin(), portalAccel.end(), '>', '+');
    if (!portalAccel.empty() && portalAccel.back() == '+') {
        portalAccel.pop_back();
    }
    return portalAccel;
}

#pragma endregion

} // namespace input