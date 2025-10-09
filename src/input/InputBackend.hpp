#ifndef INPUT_BACKEND_HPP
#define INPUT_BACKEND_HPP

#include <functional>
#include <string>

namespace input {

class InputBackend {
public:
    virtual ~InputBackend() = default;

    // Initialise le backend
    virtual bool initialize() = 0;

    // Enregistre un raccourci clavier
    // name: nom unique du raccourci
    // accelerator: combinaison de touches au format GTK (ex: "<Control>s")
    // callback: fonction à appeler lorsque le raccourci est activé
    virtual void registerShortcut(const std::string& name, const std::string& accelerator,
                                  std::function<void()> callback) = 0;

    // Désenregistre un raccourci
    virtual void unregisterShortcut(const std::string& name) = 0;

    // Vérifie si le backend est disponible
    virtual bool isAvailable() const = 0;

    // État de la session
    enum class SessionState {
        DISCONNECTED,  // Non connecté
        CONNECTING,    // Connexion en cours
        CONNECTED,     // Connecté et opérationnel
        ERROR          // Erreur de connexion
    };
};

}  // namespace input

#endif  // INPUT_BACKEND_HPP
