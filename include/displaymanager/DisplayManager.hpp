#ifndef DISPLAY_MANAGER_HPP
#define DISPLAY_MANAGER_HPP

#include <vector>
#include <memory>
#include <string>
#include <cstdint>  // Pour les types de taille fixe

struct DisplayInfo {
    int32_t id;         // Identifiant unique de l'écran
    int32_t x;          // Position X de l'écran
    int32_t y;          // Position Y de l'écran
    uint32_t width;     // Largeur de l'écran en pixels
    uint32_t height;    // Hauteur de l'écran en pixels
    float scale;        // Facteur d'échelle de l'écran
    bool isPrimary;     // Indique si c'est l'écran principal
    std::string name;   // Nom de l'écran (fourni par le système)
    void* ptr;          // Pointeur vers les données spécifiques à la plateforme
    
    // Constructeur par défaut
    DisplayInfo() 
        : id(-1), 
          x(0), 
          y(0), 
          width(0), 
          height(0), 
          scale(1.0f), 
          isPrimary(false),
          ptr(nullptr)
    {}
    
    // Constructeur avec paramètres
    DisplayInfo(int32_t id, int32_t x, int32_t y, uint32_t w, uint32_t h, 
                float s = 1.0f, bool primary = false, const std::string& n = "")
        : id(id), 
          x(x), 
          y(y), 
          width(w), 
          height(h), 
          scale(s), 
          isPrimary(primary), 
          name(n),
          ptr(nullptr)
    {}
    
    // Opérateur d'égalité
    bool operator==(const DisplayInfo& other) const {
        return id == other.id && 
               x == other.x && 
               y == other.y && 
               width == other.width && 
               height == other.height && 
               isPrimary == other.isPrimary;
    }
    
    // Vérifie si un point est contenu dans cet écran
    bool contains(int32_t px, int32_t py) const {
        return px >= x && px < static_cast<int32_t>(x + width) &&
               py >= y && py < static_cast<int32_t>(y + height);
    }
};

/**
 * @class DisplayManager
 * @brief Gère les écrans et les configurations d'affichage
 * 
 * Cette classe fournit une interface pour interagir avec les écrans connectés,
 * obtenir des informations sur leur disposition, et gérer les changements de configuration.
 */
class DisplayManager {
public:
    /**
     * @brief Constructeur par défaut
     */
    DisplayManager();
    
    /**
     * @brief Destructeur
     */
    ~DisplayManager();
    
    // Empêcher la copie et l'affectation
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    /**
     * @brief Initialise le gestionnaire d'écrans
     * @return true si l'initialisation a réussi, false sinon
     */
    bool initialize();
    
    /**
     * @brief Met à jour la liste des écrans disponibles
     * @throw std::runtime_error en cas d'erreur lors de la mise à jour
     */
    void updateDisplays();
    
    /**
     * @brief Obtient la liste des écrans disponibles
     * @return Une référence constante vers le vecteur d'écrans
     */
    const std::vector<DisplayInfo>& getDisplays() const;
    
    /**
     * @brief Alias pour getDisplays() pour la rétrocompatibilité
     */
    const std::vector<DisplayInfo>& getAvailableDisplays() const { return getDisplays(); }
    
    /**
     * @brief Obtient l'écran principal
     * @return Un pointeur vers l'écran principal, ou nullptr si non trouvé
     */
    const DisplayInfo* getPrimaryDisplay() const;
    
    /**
     * @brief Trouve l'écran contenant les coordonnées spécifiées
     * @param windowX Coordonnée X de la fenêtre
     * @param windowY Coordonnée Y de la fenêtre
     * @return Un pointeur vers l'écran contenant le point, ou nullptr si aucun écran ne le contient
     */
    const DisplayInfo* getDisplayForWindow(int windowX, int windowY) const;
    
    /**
     * @brief Active ou désactive le suivi de la souris
     * @param enable true pour activer, false pour désactiver
     */
    void setFollowMouse(bool enable);
    
    /**
     * @brief Vérifie si le suivi de la souris est activé
     * @return true si le suivi est activé, false sinon
     */
    bool isFollowingMouse() const;
    
    /**
     * @brief Configure la taille de la fenêtre
     * @param width Largeur de la fenêtre
     * @param height Hauteur de la fenêtre
     */
    void setWindowSize(int width, int height);
    
    /**
     * @brief Active ou désactive le mode plein écran
     * @param fullscreen true pour activer le plein écran, false sinon
     */
    void setFullscreen(bool fullscreen);
    
    /**
     * @brief Commence une nouvelle frame de rendu
     */
    void beginFrame();
    
    /**
     * @brief Termine la frame de rendu actuelle
     */
    void endFrame();

private:
    // Implémentation PIMPL
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // DISPLAY_MANAGER_HPP
