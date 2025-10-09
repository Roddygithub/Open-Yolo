// Inclure d'abord les en-têtes standards C++
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

// Définir _GNU_SOURCE si nécessaire
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Désactiver les avertissements pour GTK/GLib
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Inclure les en-têtes GTK/GTKmm
#include <glib.h>
#include <glibmm.h>
#include <glibmm/init.h>
#include <gdk/gdkwayland.h>
#include <gdkmm.h>
#include <giomm.h>
#include <gtkmm.h>

// Réactiver les avertissements
#pragma GCC diagnostic pop

// Sauvegarder les définitions de macros problématiques
#pragma push_macro("None")
#pragma push_macro("Bool")
#pragma push_macro("Status")
#pragma push_macro("Success")
#pragma push_macro("Always")
#pragma push_macro("False")
#pragma push_macro("True")

// Désactiver les macros problématiques
#undef None
#undef Bool
#undef Status
#undef Success
#undef Always
#undef False
#undef True

// Enfin nos en-têtes
#include "../../include/input/backends/WaylandBackend.hpp"

namespace input {

// Constantes pour la gestion des reconnexions et des délais
constexpr int MAX_RETRIES = 3;
constexpr int RETRY_DELAY_SEC = 5;
constexpr int DEBOUNCE_DELAY_MS = 100;

// Constantes pour D-Bus
constexpr const char* DBUS_SERVICE = "org.freedesktop.portal.Desktop";
constexpr const char* DBUS_PATH = "/org/freedesktop/portal/desktop";
constexpr const char* DBUS_INTERFACE = "org.freedesktop.portal.GlobalShortcuts";

// Définition des macros de journalisation
#ifndef LOG_INFO
#define LOG_INFO(msg) do { std::cout << "[INFO] " << (msg) << std::endl; } while(0)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(msg) do { std::cerr << "[ERROR] " << (msg) << std::endl; } while(0)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(msg) do { std::cout << "[DEBUG] " << (msg) << std::endl; } while(0)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(msg) do { std::cerr << "[WARNING] " << (msg) << std::endl; } while(0)
#endif

// Définition des macros de journalisation avec protection contre les effets de bord
#ifndef LOG_INFO
#define LOG_INFO(msg) do { std::cout << "[INFO] " << (msg) << std::endl; } while(0)
#endif
#ifndef LOG_ERROR
#define LOG_ERROR(msg) do { std::cerr << "[ERROR] " << (msg) << std::endl; } while(0)
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(msg) do { std::cout << "[DEBUG] " << (msg) << std::endl; } while(0)
#endif
#ifndef LOG_WARNING
#define LOG_WARNING(msg) do { std::ostringstream oss; oss << "[WARNING] " << msg; std::cerr << oss.str() << std::endl; } while(0)
#endif

WaylandBackend::WaylandBackend(InputManager* manager)
    : m_manager(manager),
      m_window(nullptr),
      m_initialized(false),
      m_sessionState(WaylandBackend::SessionState::Disconnected),
      m_retryCount(0) {
    LOG_INFO("Initialisation du backend Wayland...");
}

WaylandBackend::~WaylandBackend() {
    LOG_INFO("Nettoyage du backend Wayland...");
    if (this->m_updateTimer.connected()) {
        this->m_updateTimer.disconnect();
    }
    this->closeSession();
}

void input::WaylandBackend::setWindow(Gtk::Window* window) {
    this->m_window = window;
    std::cout << "[WaylandBackend] Fenêtre associée : "
              << (this->m_window ? "OK" : "NULL") << std::endl;
    
    if (this->m_window) {
        auto display = this->m_window->get_display();
        if (GDK_IS_WAYLAND_DISPLAY(display->gobj())) {
            std::cout << "[WaylandBackend] Connexion au display Wayland..." << std::endl;
        }
    }
}

void WaylandBackend::setSessionState(SessionState state) {
    if (this->m_sessionState == state) {
        return;
    }

    SessionState oldState = this->m_sessionState;
    this->m_sessionState = state;
    
    std::cout << "[WaylandBackend] Changement d'état de session : ";
    
    switch (this->m_sessionState) {
        case SessionState::Disconnected: 
            std::cout << "Déconnecté"; 
            break;
        case SessionState::Connecting:   
            std::cout << "Connexion en cours..."; 
            break;
        case SessionState::Connected:    
            std::cout << "Connecté"; 
            break;
        case SessionState::Reconnecting: 
            std::cout << "Reconnexion..."; 
            break;
    }
    
    std::cout << std::endl;
    
    // Notifier les observateurs du changement d'état
    for (const auto& callback : m_stateChangeCallbacks) {
        if (callback) {
            callback(oldState, m_state);
        }
    }
}

bool WaylandBackend::onKeyPressed(GdkEventKey* event) {
    if (!event) {
        return false;
    }
    
    std::cout << "[WaylandBackend] Touche pressée : "
              << gdk_keyval_name(event->keyval)
              << " (keycode: " << event->keyval << ")"
              << std::endl;
    
    // Ici, vous pouvez ajouter la logique de gestion des raccourcis
    // Par exemple, vérifier si cette touche correspond à un raccourci enregistré
    
    return false; // Retourne true si l'événement a été traité, false sinon
}

void input::WaylandBackend::onSessionCreated(Glib::RefPtr<Gio::AsyncResult>& result) {
    std::cout << "[WaylandBackend] Session Wayland créée (AsyncResult reçu)" << std::endl;
    
    try {
        // Essayer de récupérer le résultat de la création de session
        // et effectuer les opérations nécessaires
        
        // Exemple : marquer la session comme connectée
        this->setSessionState(WaylandBackend::SessionState::Connected);
        
    } catch (const Glib::Error& error) {
        std::cerr << "[WaylandBackend] Erreur lors de la création de la session: " 
                  << error.what() << std::endl;
        this->setSessionState(WaylandBackend::SessionState::Disconnected);
    }
}

// Implémentation des méthodes de gestion des erreurs
void WaylandBackend::handleError(const std::string& message, const Glib::Error* e) {
    std::string full_message = "[WaylandBackend] " + message;
    if (e) {
        full_message += ": " + std::string(e->what());
    }
    std::cerr << full_message << std::endl;
    
    // Appeler les callbacks d'erreur
    for (const auto& cb : m_errorCallbacks) {
        cb(full_message);
    }
}

void WaylandBackend::handleError(const std::string& message, const std::exception* e) {
    std::string full_message = "[WaylandBackend] " + message;
    if (e) {
        full_message += ": " + std::string(e->what());
    }
    std::cerr << full_message << std::endl;
    
    // Appeler les callbacks d'erreur
    for (const auto& cb : m_errorCallbacks) {
        cb(full_message);
    }
}

bool input::WaylandBackend::isProxyValid() const {
    return this->m_proxy && !this->m_proxy->get_name_owner().empty();
}

bool input::WaylandBackend::initialize() {
    this->m_retryCount = 0;
    auto gdkDisplay = Gdk::Display::get_default();
    if (!gdkDisplay || !GDK_IS_WAYLAND_DISPLAY(gdkDisplay->gobj())) {
        this->handleError("Le backend Wayland ne peut être initialisé que sur un affichage Wayland.");
        return false;
    }
    this->setSessionState(WaylandBackend::SessionState::Connecting);
    try {
        this->m_dbusConnection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    } catch (const Glib::Error& e) {
        this->handleError("Erreur de connexion au bus D-Bus de session", &e);
        this->setSessionState(WaylandBackend::SessionState::Disconnected);
        return false;
    }
    Gio::DBus::Proxy::create(
        this->m_dbusConnection,
        input::DBUS_SERVICE, input::DBUS_PATH, input::DBUS_INTERFACE,
        [this](Glib::RefPtr<Gio::AsyncResult>& result) { this->onProxyCreated(result); }
    );
    this->m_initialized = true;
    LOG_INFO("Initialisation du backend Wayland (via D-Bus Portal) en cours...");
    return true;
}

void input::WaylandBackend::onProxyCreated(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        this->m_proxy = Gio::DBus::Proxy::create_finish(result);
        if (!this->isProxyValid()) {
            throw std::runtime_error("Le service de portail XDG n'est pas disponible. Vérifiez que xdg-desktop-portal est installé et en cours d'exécution.");
        }
        LOG_INFO("Proxy D-Bus pour GlobalShortcuts créé. Fourni par: " + this->m_proxy->get_name_owner());
        this->createPortalSession();
    } catch (const std::runtime_error& e) {
        this->handleError("Erreur lors de la création du proxy D-Bus", &e);
        this->setSessionState(WaylandBackend::SessionState::Reconnecting);
        this->scheduleReconnect();
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

void input::WaylandBackend::closeSession() {
    if (this->m_sessionHandle.empty()) return;

    if (!this->isProxyValid()) {
        LOG_WARNING("Proxy D-Bus non valide, impossible de fermer proprement la session.");
    } else {
        try {
            this->m_proxy->call("CloseSession", Glib::VariantContainerBase::create_tuple({ Glib::Variant<Glib::ustring>::create(this->m_sessionHandle) }));
        } catch (const std::exception& e) {
            this->handleError("Erreur (ignorée) lors de la demande de fermeture de la session D-Bus", &e);
        }
    }
    this->m_sessionHandle.clear();
    this->m_sessionProxy.reset();
    this->setSessionState(WaylandBackend::SessionState::Disconnected);
}

// Délai de 100ms
void input::WaylandBackend::scheduleUpdate() {
    // Si une mise à jour est déjà planifiée, on ne fait rien
    if (this->m_updateTimer.connected()) {
        return;
    }
    
    // Planifier la mise à jour avec un délai pour regrouper les changements
    this->m_updateTimer = Glib::signal_timeout().connect(
        [this]() {
            if (this->m_sessionState == WaylandBackend::SessionState::Connected) {
                try {
                    if (this->m_shortcuts.empty()) {
                        this->closeSession();
                    } else {
                        this->recreateSession();
                    }
                } catch (const std::exception& e) {
                    this->handleError("Erreur lors de la mise à jour de la session", &e);
                }
            }
            return false; // Ne pas répéter le timer
        },
        input::DEBOUNCE_DELAY_MS
    );
}

bool input::WaylandBackend::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    if (this->m_sessionState != WaylandBackend::SessionState::Connected) {
        LOG_WARNING("Cannot register shortcut: Not connected to session");
        return false;
    }

    if (name.empty()) {
        this->handleError("Shortcut name cannot be empty");
        return false;
    }

    if (!callback) {
        this->handleError("Callback for shortcut '" + name + "' cannot be null");
        return false;
    }

    try {
        // Convert accelerator to portal format
        std::string portalShortcut = this->convertToPortalShortcut(accelerator);
        if (portalShortcut.empty()) {
            LOG_ERROR("Failed to convert shortcut: " + accelerator);
            return false;
        }

        // Check if shortcut already exists
        if (this->m_shortcuts.find(name) != this->m_shortcuts.end()) {
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

        // Enregistrer le raccourci
        this->m_proxy->call_sync("Call", params);
        
        // Stocker le callback
        this->m_shortcuts[name] = { accelerator, callback };
        
        LOG_DEBUG("Registered shortcut: " + name + " -> " + accelerator);
        return true;
    } catch (const Glib::Error& e) {
        this->handleError("Failed to register shortcut: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        this->handleError("Unexpected error while registering shortcut", &e);
        return false;
    }
}

std::string input::WaylandBackend::convertToPortalShortcut(const std::string& gtkAccel) {
    // This is a basic implementation and might need adjustments
    std::string result = gtkAccel;
    
    // Convert key names to lowercase for portal
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    // Replace key names if needed
    // Example: "Control" -> "Ctrl"
    size_t pos = 0;
    while ((pos = result.find("control", pos)) != std::string::npos) {
        result.replace(pos, 7, "ctrl");
        pos += 4;
    }
    
    // Clean up any extra spaces
    result.erase(std::unique(result.begin(), result.end(),
        [](char a, char b) { return a == ' ' && b == ' '; }), result.end());
    
    return result;
}

void input::WaylandBackend::addErrorCallback(WaylandBackend::ErrorCallback&& callback) {
    if (!callback) {
        this->handleError("Callback cannot be null");
        return;
    }
    m_errorCallbacks.push_back(std::move(callback));
}

void input::WaylandBackend::onBindShortcutsResponse(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        this->m_proxy->call_finish(result);
        this->setSessionState(WaylandBackend::SessionState::Connected);
    } catch (const Glib::Error& e) {
        this->handleError("Erreur lors de la liaison des raccourcis", &e);
        this->setSessionState(WaylandBackend::SessionState::Reconnecting);
        this->scheduleReconnect();
    } catch (const std::exception& e) {
        this->handleError("Erreur inattendue lors de la liaison des raccourcis", &e);
        this->setSessionState(WaylandBackend::SessionState::Reconnecting);
        this->scheduleReconnect();
    }
}

void input::WaylandBackend::recreateSession() {
    try {
        // Fermer la session existante si elle est ouverte
        this->closeSession();
        
        // Recréer la session
        this->createPortalSession();
        
        // Réenregistrer tous les raccourcis
        auto shortcutsCopy = this->m_shortcuts;
        this->m_shortcuts.clear();
        
        for (const auto& [name, shortcut] : shortcutsCopy) {
            if (!this->registerShortcut(name, shortcut.accelerator, shortcut.callback)) {
                this->handleError("Échec du réenregistrement du raccourci: " + name);
            }
        }
        
        LOG_INFO("Session recréée avec succès");
    } catch (const std::exception& e) {
        this->handleError("Erreur lors de la recréation de la session", &e);
        this->scheduleReconnect();
    }
}

void input::WaylandBackend::scheduleReconnect() {
    if (this->m_retryCount >= input::MAX_RETRIES) {
        this->handleError("Nombre maximum de tentatives de reconnexion atteint");
        this->setSessionState(WaylandBackend::SessionState::Disconnected);
        return;
    }

    this->m_retryCount++;
    std::ostringstream oss;
    oss << "Tentative de reconnexion " << this->m_retryCount << "/" << input::MAX_RETRIES;
    LOG_WARNING(oss.str());
    
    this->m_updateTimer = Glib::signal_timeout().connect(
        sigc::bind_return(
            sigc::mem_fun(*this, &WaylandBackend::initialize),
            false // Ne pas répéter
        ),
        input::RETRY_DELAY_SEC * 1000 // Convertir en millisecondes
    );
}
void input::WaylandBackend::onSessionShortcutActivated(const Glib::ustring& sender_name,
                                                      const Glib::ustring& signal_name,
                                                      const Glib::VariantContainerBase& parameters) {
    if (signal_name != "GlobalShortcutFired" || parameters.get_n_children() < 2) return;
    try {
        Glib::Variant<Glib::ustring> session_handle, shortcut_id;
        parameters.get_child(session_handle, 0);
        parameters.get_child(shortcut_id, 1);
        
        if (session_handle.get() != this->m_sessionHandle) return;
        
        auto it = this->m_shortcuts.find(shortcut_id.get());
        if (it != this->m_shortcuts.end()) {
            it->second.callback();
        }
    } catch (const Glib::Error& e) {
        this->handleError("Erreur lors du traitement du signal GlobalShortcutFired", &e);
    }
}


// Restaurer les définitions de macros
#pragma pop_macro("None")
#pragma pop_macro("Bool")
#pragma pop_macro("Status")
#pragma pop_macro("Success")
#pragma pop_macro("Always")
#pragma pop_macro("False")
#pragma pop_macro("True")
} // namespace input