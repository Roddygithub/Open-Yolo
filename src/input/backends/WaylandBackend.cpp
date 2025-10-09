// Inclure d'abord les en-têtes du projet
#include "../../include/input/backends/WaylandBackend.hpp"
#include "../InputManager.h"

// Inclure les en-têtes GTK/GTKmm
#include <gdk/gdkwayland.h>
#include <gdkmm.h>
#include <giomm.h>
#include <glibmm.h>
#include <gtkmm.h>

// Inclure les en-têtes standards C++
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Désactiver les avertissements pour GTK/GLib
#pragma GCC diagnostic push
#include <glib.h>
#pragma GCC diagnostic pop

// Sauvegarder les définitions de macros problématiques
#undef Always
#undef False
#undef True
// Définition des macros de journalisation
#define LOG_DEBUG(msg) do { std::cout << "[DEBUG] " << (msg) << std::endl; } while(0)
#define LOG_INFO(msg) do { std::cout << "[INFO] " << (msg) << std::endl; } while(0)
#define LOG_ERROR(msg) do { std::cerr << "[ERROR] " << (msg) << std::endl; } while(0)

namespace input {

// Constantes pour la configuration
namespace {
    constexpr int MAX_RETRIES = 5;
    constexpr const char* DBUS_SERVICE = "org.freedesktop.portal.Desktop";
    constexpr const char* DBUS_PATH = "/org/freedesktop/portal/desktop";
    constexpr const char* DBUS_INTERFACE = "org.freedesktop.portal.GlobalShortcuts";
}

WaylandBackend::WaylandBackend(InputManager* manager)
    : m_manager(manager)
    , m_window(nullptr)
    , m_minLogLevel(LogLevel::Info)
    , m_retryCount(0)
    , m_sessionState(SessionState::Disconnected)
    , m_initialized(false) {
    LOG_INFO("Initialisation du backend Wayland...");
    
    // Initialiser la connexion DBus
    try {
        initialize();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize Wayland backend: " + std::string(e.what()));
        // Planifier une nouvelle tentative
        scheduleReconnect();
    }
}

void WaylandBackend::setSessionState(SessionState state) {
    if (m_sessionState == state) {
        return;
    }
    
    SessionState oldState = m_sessionState;
    m_sessionState = state;
    
    // Log du changement d'état
    std::cout << "Changement d'état: ";
    switch (state) {
        case SessionState::Disconnected: 
            std::cout << "Déconnecté"; 
            break;
        case SessionState::Connecting:   
            std::cout << "Connexion en cours"; 
            break;
        case SessionState::Connected:    
            std::cout << "Connecté"; 
            break;        
        case SessionState::Reconnecting:
            std::cout << "Reconnexion en cours";
            break;
    }
    std::cout << std::endl;

    // Notifier les observateurs du changement d'état
    for (const auto& callback : m_stateChangeCallbacks) {
        if (callback) {
            try {
                callback(oldState, state);
            } catch (const std::exception& e) {
                LOG_ERROR("Error in state change callback: " + std::string(e.what()));
            }
        }
    }

}

WaylandBackend::~WaylandBackend() {
    // Nettoyer les ressources
    closeSession();
    
    // Désactiver tous les timers
    if (m_updateTimer.connected()) {
        m_updateTimer.disconnect();
    }
    
    if (m_reconnectTimer.connected()) {
        m_reconnectTimer.disconnect();
    }
    
    // Libérer les ressources D-Bus
    m_proxy.reset();
    if (m_connection) {
        m_connection->close_sync();
        m_connection.reset();
    }
}

bool WaylandBackend::isAvailable() const {
    // Vérifier si nous sommes sous Wayland
    auto* display = gdk_display_get_default();
    return display && GDK_IS_WAYLAND_DISPLAY(display);
}

bool WaylandBackend::initialize() {
    m_retryCount = 0;
    auto gdkDisplay = Gdk::Display::get_default();
    if (!gdkDisplay || !GDK_IS_WAYLAND_DISPLAY(gdkDisplay->gobj())) {
        LOG_ERROR("Le backend Wayland ne peut être initialisé que sur un affichage Wayland.");
        return false;
    }
    setSessionState(SessionState::Connecting);
    try {
        m_connection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    } catch (const Glib::Error& e) {
        LOG_ERROR(std::string("Erreur de connexion au bus D-Bus de session: ") + e.what());
        setSessionState(SessionState::Disconnected);
        return false;
    }

    try {
        // Se connecter à la session DBus
        m_connection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
        if (!m_connection) {
            throw std::runtime_error("Failed to connect to session bus");
        }

        // Créer un proxy pour l'interface de raccourcis globaux
        m_proxy = Gio::DBus::Proxy::create_sync(
            m_connection,
            "org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.GlobalShortcuts"
        );
        
        if (!m_proxy) {
            throw std::runtime_error("Failed to create DBus proxy");
        }
        
        m_initialized = true;
        
        // Planifier la création de la session
        Glib::signal_idle().connect_once([this]() {
            try {
                recreateSession();
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to create session: " + std::string(e.what()));
                scheduleReconnect();
            }
        });
        
        return true;
    } catch (const Glib::Error& e) {
        throw std::runtime_error("DBus error: " + std::string(e.what()));
    }
}

void WaylandBackend::registerShortcut(const std::string& name, 
                                    const std::string& accelerator, 
                                    std::function<void()> callback) {
    m_shortcuts[name] = ShortcutData(accelerator, callback); // Stocker les informations d'accélérateur
    recreateSession();
}

void WaylandBackend::unregisterShortcut(const std::string& name) {
    if (m_shortcuts.erase(name) > 0) {
        recreateSession();
    }
}

void WaylandBackend::onSessionShortcutActivated(const Glib::ustring& sender_name,
                                               const Glib::ustring& signal_name,
                                               const Glib::VariantContainerBase& parameters) {
    (void)sender_name; // Paramètre non utilisé
    
    if (signal_name != "Activated") {
        return;
    }
    
    try {
        // Extraire les paramètres du signal
        Glib::Variant<Glib::ustring> session_handle;
        Glib::Variant<Glib::ustring> shortcut_id;
        Glib::Variant<guint64> timestamp;
        Glib::Variant<Glib::VariantBase> options;
        
        parameters.get_child(session_handle, 0);
        parameters.get_child(shortcut_id, 1);
        parameters.get_child(timestamp, 2);
        parameters.get_child(options, 3);
        
        if (session_handle.get() != m_sessionHandle) {
            return; // Ce n'est pas notre session
        }
        
        // Trouver et appeler le callback correspondant
        auto it = m_shortcuts.find(shortcut_id.get());
        if (it != m_shortcuts.end()) {
            // Vérifier si le callback est valide
            if (it->second.callback) {
                // Appel du callback
                it->second.callback();
            } else {
                LOG_ERROR("Callback invalide pour le raccourci: " + shortcut_id.get());
            }
        }
    } catch (const Glib::Error& e) {
        LOG_ERROR("Erreur lors du traitement du signal de raccourci: " + std::string(e.what()));
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du traitement du signal de raccourci: " + std::string(e.what()));
    }
    // Fin de la méthode
}

void WaylandBackend::onBindShortcutsResponse(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        // Vérifier la réponse de la méthode BindShortcuts
        auto reply = m_proxy->call_finish(result);
        
        // Extraire le handle de session
        Glib::Variant<Glib::ustring> session_handle;
        reply.get_child(session_handle, 0);
        
        m_sessionHandle = session_handle.get();
        setSessionState(SessionState::Connected);
        m_retryCount = 0; // Réinitialiser le compteur de tentatives
        
        // S'abonner au signal d'activation des raccourcis
        m_proxy->signal_signal().connect(
            sigc::mem_fun(*this, &WaylandBackend::onSessionShortcutActivated)
        );
        
    } catch (const Glib::Error& e) {
        LOG_ERROR("Failed to bind shortcuts: " + std::string(e.what()));
        setSessionState(SessionState::Disconnected);
        scheduleReconnect();
    }
}

void WaylandBackend::recreateSession() {
    if (m_sessionState == SessionState::Connecting) {
        return; // Déjà en cours de connexion
    }
    
    setSessionState(SessionState::Connecting);
    
    try {
        // Créer une nouvelle session
        auto parameters = Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create("openyolo"),
            Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>::create({})
        });
        
        m_proxy->call(
            "CreateSession",
            sigc::mem_fun(*this, &WaylandBackend::onSessionCreated),
            parameters
        );
        
    } catch (const Glib::Error& e) {
        LOG_ERROR("Failed to create session: " + std::string(e.what()));
        setSessionState(SessionState::Disconnected);
        scheduleReconnect();
    }
}

void WaylandBackend::onSessionCreated(Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        // Vérifier la réponse de la méthode CreateSession
        auto reply = m_proxy->call_finish(result);
        
        // Extraire le handle de session
        Glib::Variant<Glib::ustring> session_handle;
        reply.get_child(session_handle, 0);
        
        // Enregistrer les raccourcis existants
        // bindExistingShortcuts(session_handle.get());
        
    } catch (const Glib::Error& e) {
        LOG_ERROR("Failed to create session: " + std::string(e.what()));
        setSessionState(SessionState::Disconnected);
        scheduleReconnect();
    }
}

void WaylandBackend::bindExistingShortcuts(const Glib::ustring& session_handle) {
    if (m_shortcuts.empty()) {
        setSessionState(SessionState::Connected);
        return;
    }
    
    try {
        // Préparer la liste des raccourcis à enregistrer
        std::vector<std::map<Glib::ustring, Glib::VariantBase>> shortcuts;
        
        for (const auto& [name, _] : m_shortcuts) {
            // Convertir le nom en accélérateur (simplifié pour l'exemple)
            std::string accelerator = name; // À remplacer par la vraie conversion
            std::string portal_shortcut = convertToPortalShortcut(accelerator);
            
            if (!portal_shortcut.empty()) {
                shortcuts.push_back({
                    {"shortcut", Glib::Variant<Glib::ustring>::create(portal_shortcut)},
                    {"description", Glib::Variant<Glib::ustring>::create(name)}
                });
            }
        }
        
        // Appeler la méthode BindShortcuts
        auto parameters = Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create(session_handle),
            Glib::Variant<Glib::ustring>::create("openyolo"),
            Glib::Variant<decltype(shortcuts)>::create(shortcuts),
            Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>::create({})
        });
        
        m_proxy->call(
            "BindShortcuts",
            sigc::mem_fun(*this, &WaylandBackend::onBindShortcutsResponse),
            parameters
        );
        
    } catch (const Glib::Error& e) {
        LOG_ERROR("Failed to bind shortcuts: " + std::string(e.what()));
        setSessionState(SessionState::Disconnected);
        scheduleReconnect();
    }
}

void WaylandBackend::closeSession() {
    if (m_updateTimer.connected()) {
        m_updateTimer.disconnect();
    }

    if (m_reconnectTimer.connected()) {
        m_reconnectTimer.disconnect();
    }

    if (m_sessionState == SessionState::Connected && !m_sessionHandle.empty()) {
        try {
            std::vector<Glib::VariantBase> params = {
                Glib::Variant<Glib::ustring>::create(m_sessionHandle)
            };
            auto parameters = Glib::VariantContainerBase::create_tuple(params);
            if (m_proxy) {
                m_proxy->call_sync("CloseSession", parameters);
            }
        } catch (const Glib::Error& e) {
            LOG_ERROR("Failed to close session: " + std::string(e.what()));
        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Failed to close session: ") + e.what());
        }
    }

    m_sessionHandle.clear();
    setSessionState(SessionState::Disconnected);
}

void WaylandBackend::scheduleReconnect() {
    if (m_reconnectTimer.connected()) {
        return; // Une reconnexion est déjà programmée
    }

    //Augmenter le compteur de tentatives
    m_retryCount++;
    const int max_retry_delay = 30000; // 30 secondes max
    int delay_ms = std::min(1000 * (1 << std::min(m_retryCount, 5)), max_retry_delay);

    LOG_INFO("Scheduling reconnect in " + std::to_string(delay_ms) + "ms (attempt " + std::to_string(m_retryCount) + ")");

    // Utiliser une copie locale de this pour la capture
    WaylandBackend* self = this;
    m_reconnectTimer = Glib::signal_timeout().connect(
        [self]() -> bool {
            try {
                if (self->m_reconnectTimer.connected()) {
                    self->m_reconnectTimer.disconnect();
                }
                self->initialize();
                return false; // Ne pas répéter
            } catch (const std::exception& e) {
                LOG_ERROR("Reconnect failed: " + std::string(e.what()));
                self->scheduleReconnect(); // Réessayer
                return false;
            }
        },
        delay_ms
    );
}

std::string WaylandBackend::convertToPortalShortcut(const std::string& gtkAccel) {
    // Table de conversion des modificateurs
    static const std::unordered_map<std::string, std::string> mod_map = {
        {"<Control>", "<Ctrl>"},
        {"<Primary>", "<Super>"}, // Traduire Primary en Super pour le portail
        {"<Alt>", "<Alt>"},
        {"<Shift>", "<Shift>"},
        {"<Super>", "<Super>"},
        {"<Meta>", "<Super>"}
    };
    
    // Table de conversion des touches spéciales
    static const std::unordered_map<std::string, std::string> key_map = {
        {"Return", "Return"},
        {"Escape", "Escape"},
        {"space", "space"},
        {"Tab", "Tab"},
        {"BackSpace", "Backspace"},
        {"Up", "Up"},
        {"Down", "Down"},
        {"Left", "Left"},
        {"Right", "Right"},
        {"F1", "F1"},
        {"F2", "F2"},
        // Ajouter d'autres touches spéciales au besoin
    };
    
    // Parser l'accélérateur
    guint keyval = 0;
    GdkModifierType mods = GdkModifierType(0);
    
    gtk_accelerator_parse(gtkAccel.c_str(), &keyval, &mods);
    if (keyval == 0) {
        LOG_ERROR("Failed to parse accelerator: " + gtkAccel);
        return "";
    }
    
    // Convertir la touche
    gchar* keyname = gdk_keyval_name(keyval);
    if (!keyname) {
        LOG_ERROR("Failed to get key name for keyval: " + std::to_string(keyval));
        return "";
    }
    
    std::string key = keyname;
    auto key_it = key_map.find(key);
    if (key_it != key_map.end()) {
        key = key_it->second;
    } else {
        // Convertir en minuscules pour les touches alphabétiques
        if (key.length() == 1 && std::isalpha(key[0])) {
            key = std::string(1, std::tolower(key[0]));
        }
    }
    
    // Construire la chaîne du raccourci
    std::string result;
    
    if (mods & GDK_CONTROL_MASK) {
        result += "<Ctrl>";
    }
    if (mods & GDK_SHIFT_MASK) {
        if (!result.empty()) result += "+";
        result += "<Shift>";
    }
    if (mods & GDK_MOD1_MASK) {  // GDK_MOD1_MASK correspond à la touche Alt
        if (!result.empty()) result += "+";
        result += "<Alt>";
    }
    if (mods & GDK_SUPER_MASK) {
        if (!result.empty()) result += "+";
        result += "<Super>";
    }
    
    if (!result.empty()) {
        result += "+" + key;
    } else {
        result = key;
    }
    
    return result;
}

void WaylandBackend::handleError(const std::string& message, const Glib::Error* e) {
    std::string errorMsg = message;
    if (e) {
        errorMsg += ": ";
        errorMsg += e->what();
    }
    LOG_ERROR(errorMsg);
    
    // Appeler les callbacks d'erreur enregistrés
    for (const auto& callback : m_errorCallbacks) {
        callback(errorMsg);
    }
}

void WaylandBackend::handleError(const std::string& message, const std::exception* e) {
    std::string errorMsg = message;
    if (e) {
        errorMsg += ": ";
        errorMsg += e->what();
    }
    LOG_ERROR(errorMsg);
    
    // Appeler les callbacks d'erreur enregistrés
    // (à implémenter si nécessaire)
}

bool WaylandBackend::handleKeyEvent(guint keyval, GdkModifierType mods)
{
    if (!m_initialized) {
        LOG_DEBUG("WaylandBackend not initialized, ignoring key event");
        return false;
    }

    // Parcourir tous les raccourcis enregistrés
    for (const auto& [name, shortcut] : m_shortcuts) {
        // Vérifier si la touche et les modificateurs correspondent
        if (shortcut.keyval == keyval && 
            shortcut.mods == (mods & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK))) {
            // Appeler le callback du raccourci
            if (shortcut.callback) {
                LOG_DEBUG("Triggering shortcut: " + name);
                shortcut.callback();
                return true;
            }
        }
    }
    
    return false;
}

bool WaylandBackend::onKeyPressed(GdkEventKey* event)
{
    if (!m_initialized || !event) {
        return false;
    }
    
    // Convertir l'événement GdkEventKey en keyval et mods
    guint keyval = event->keyval;
    GdkModifierType mods = static_cast<GdkModifierType>(event->state);
    
    // Appeler handleKeyEvent avec les paramètres convertis
    return handleKeyEvent(keyval, mods);
}

bool WaylandBackend::shutdown() {
    try {
        LOG_INFO("Arrêt du backend Wayland...");
        
        // Fermer la session D-Bus si elle est ouverte
{{ ... }}
        
        // Réinitialiser l'état
        m_initialized = false;
        m_retryCount = 0;
        
        // Vider les maps de raccourcis si elles existent
        if (!m_shortcuts.empty()) {
            m_shortcuts.clear();
        }
        
        // Vider les raccourcis en attente si la map existe
        if (!m_pendingShortcuts.empty()) {
            m_pendingShortcuts.clear();
        }
        
        // Annuler toute reconnexion planifiée
        if (m_reconnectConnection.connected()) {
            m_reconnectConnection.disconnect();
        }
        
        // Fermer la connexion D-Bus si elle existe
        if (m_connection) {
            m_connection->close_sync();
            m_connection.reset();
        }
        
        setSessionState(SessionState::Disconnected);
        LOG_INFO("Backend Wayland arrêté avec succès");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de l'arrêt du backend Wayland: " + std::string(e.what()));
        return false;
    } catch (...) {
        LOG_ERROR("Erreur inconnue lors de l'arrêt du backend Wayland");
        return false;
    }
}

} // namespace input
