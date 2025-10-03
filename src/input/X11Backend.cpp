#include "../../../include/input/backends/X11Backend.hpp"
#include <iostream>
#include <gdk/gdkx.h> // Pour les fonctions spécifiques à X11
#include <X11/Xlib.h>

namespace input {

// Variable globale pour stocker l'affichage X11, nécessaire pour le gestionnaire d'erreurs.
static Display* g_x11_display = nullptr;

// Gestionnaire d'erreurs X11 personnalisé pour ignorer les erreurs "BadAccess"
// qui peuvent survenir si un autre programme a déjà capturé la même touche.
static int x11ErrorHandler(Display* dsp, XErrorEvent* error) {
    if (error->error_code == BadAccess) {
        char error_text[1024];
        XGetErrorText(dsp, error->error_code, error_text, sizeof(error_text));
        std::cerr << "Erreur X11 (BadAccess) : Impossible de capturer la touche. "
                  << "Un autre programme l'utilise probablement déjà. Erreur: " << error_text << std::endl;
        return 0; // Ignorer l'erreur
    }
    // Pour les autres erreurs, utiliser le gestionnaire par défaut.
    return XDefaultErrorHandler(dsp, error);
}

X11Backend::X11Backend() : m_xDisplay(nullptr), m_rootWindow(0) {
    std::cout << "Constructeur de X11Backend" << std::endl;
}

X11Backend::~X11Backend() {
    std::cout << "Destructeur de X11Backend" << std::endl;
    if (m_xDisplay) {
        // Libérer toutes les captures de touches avant de fermer
        XUngrabKey(m_xDisplay, AnyKey, AnyModifier, m_rootWindow);
        XSync(m_xDisplay, False);
        // Restaurer le gestionnaire d'erreurs par défaut
        XSetErrorHandler(nullptr);
    }
}

bool X11Backend::initialize() {
    auto gdkDisplay = Gdk::Display::get_default();
    if (!gdkDisplay || !GDK_IS_X11_DISPLAY(gdkDisplay->gobj())) {
        std::cerr << "Erreur: Le backend X11 a été initialisé mais l'affichage n'est pas X11." << std::endl;
        return false;
    }

    m_xDisplay = gdk_x11_display_get_xdisplay(gdkDisplay->gobj());
    if (!m_xDisplay) {
        std::cerr << "Erreur: Impossible d'obtenir l'affichage X11 depuis GDK." << std::endl;
        return false;
    }

    g_x11_display = m_xDisplay; // Stocker pour le gestionnaire d'erreurs
    m_rootWindow = DefaultRootWindow(m_xDisplay);

    // Installer notre gestionnaire d'erreurs pour gérer les conflits de raccourcis
    XSetErrorHandler(x11ErrorHandler);
    XSync(m_xDisplay, False); // S'assurer que le gestionnaire d'erreurs est bien installé

    std::cout << "Initialisation du backend X11 terminée." << std::endl;
    return true;
}

bool X11Backend::registerShortcut(const std::string& name, const std::string& accelerator, const KeyCallback& callback) {
    if (!m_xDisplay) return false;

    // Parser l'accélérateur pour obtenir keyval et modificateurs
    guint keyval = 0;
    GdkModifierType mods_type = GdkModifierType(0);
    gtk_accelerator_parse(accelerator.c_str(), &keyval, &mods_type);

    if (keyval == 0) {
        std::cerr << "Erreur de parsing de l'accélérateur X11: " << accelerator << std::endl;
        return false;
    }

    // Vérification des doublons
    auto key_pair = std::make_pair(keyval, mods_type);
    if (m_shortcuts.find(key_pair) != m_shortcuts.end()) {
        std::cerr << "Avertissement: Le raccourci '" << name << "' (" << accelerator << ") est déjà enregistré." << std::endl;
        return false; // Ou `true` si on considère que c'est un succès. `false` est plus strict.
    }

    // Stocker le callback
    m_shortcuts[key_pair] = callback;

    // Convertir le keysym en keycode
    KeyCode keycode = XKeysymToKeycode(m_xDisplay, keyval);
    if (keycode == 0) {
        std::cerr << "Erreur: Impossible de trouver un KeyCode pour l'accélérateur '"
                  << accelerator << "' (keyval: " << keyval << ")" << std::endl;
        return false;
    }

    // Conversion des modificateurs GDK en modificateurs X11
    unsigned int x_mods = 0;
    if (mods_type & GDK_CONTROL_MASK) x_mods |= ControlMask;
    if (mods_type & GDK_SHIFT_MASK)   x_mods |= ShiftMask;
    if (mods_type & GDK_MOD1_MASK)    x_mods |= Mod1Mask; // Alt
    if (mods_type & GDK_SUPER_MASK)  x_mods |= Mod4Mask; // Super/Windows

    // Capturer la touche pour toutes les combinaisons de Lock (NumLock, CapsLock, etc.)
    unsigned int modifiers_to_try[] = {
        x_mods,
        x_mods | LockMask,
        x_mods | Mod2Mask, // NumLock est souvent Mod2Mask
        x_mods | LockMask | Mod2Mask
    };

    for (unsigned int mod : modifiers_to_try) {
        XGrabKey(m_xDisplay, keycode, mod, m_rootWindow,
                 True, // Le propriétaire des événements
                 GrabModeAsync, GrabModeAsync);
    }

    XSync(m_xDisplay, False); // Appliquer les changements
    std::cout << "X11: Tentative de capture du raccourci '" << name << "' (KeyCode: " << keycode << ", Modifiers: " << x_mods << ")" << std::endl;
    return true;
}

bool X11Backend::unregisterShortcut(const std::string& name) {
    if (!m_xDisplay || !m_rootWindow) {
        std::cerr << "Erreur: Affichage X11 non initialisé lors de la suppression du raccourci." << std::endl;
        return false;
    }
    auto it = m_nameToShortcut.find(name);
    if (it == m_nameToShortcut.end()) {
        std::cerr << "Avertissement: Tentative de supprimer un raccourci X11 inexistant: " << name << std::endl;
        return false;
    }

    auto keyval = it->second.first;
    auto mods_type = it->second.second;

    KeyCode keycode = XKeysymToKeycode(m_xDisplay, keyval);
    if (keycode == 0) {
        // L'erreur a déjà été signalée lors de l'enregistrement, on peut juste nettoyer.
        m_shortcuts.erase(it->second);
        m_nameToShortcut.erase(it);
        return true;
    }

    // Conversion des modificateurs GDK en modificateurs X11
    unsigned int x_mods = 0;
    if (mods_type & GDK_CONTROL_MASK) x_mods |= ControlMask;
    if (mods_type & GDK_SHIFT_MASK)   x_mods |= ShiftMask;
    if (mods_type & GDK_MOD1_MASK)    x_mods |= Mod1Mask; // Alt
    if (mods_type & GDK_SUPER_MASK)  x_mods |= Mod4Mask; // Super/Windows

    int status = XUngrabKey(m_xDisplay, keycode, x_mods, m_rootWindow);
    Status syncStatus = XSync(m_xDisplay, False);
    if (status != Success) {
        std::cerr << "Erreur lors de la libération du raccourci X11 '" << name << "'." << std::endl;
        return false; // Empêche la suppression du raccourci de la map si l'opération échoue
    }
    if (syncStatus != Success) {
        std::cerr << "Erreur de synchronisation X11 après la libération du raccourci." << std::endl;
        // On ne retourne pas false ici, car la libération a pu réussir côté serveur
    }

    m_shortcuts.erase(it->second);
    m_nameToShortcut.erase(it);
    std::cout << "X11: Raccourci '" << name << "' supprimé." << std::endl;
    return true;
}

void X11Backend::setWindow(Gtk::Window* window) {
    // Pas nécessaire pour X11, car la capture est globale et ne dépend pas d'une fenêtre.
    (void)window;
}

bool X11Backend::onKeyPressed(GdkEventKey* event) {
    // Cette méthode est appelée par le signal GTK. Pour les raccourcis globaux,
    // les événements sont gérés directement via la boucle d'événements X11,
    // pas ici. On retourne false pour laisser GTK gérer l'événement.
    return false;
}

} // namespace input