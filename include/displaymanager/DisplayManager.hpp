#pragma once

#include <vector>
#include <memory>

struct DisplayInfo {
    int id;
    int x, y;
    int width, height;
    float scale;
    bool isPrimary;
    std::string name;
    void* ptr;  // Pointeur vers l'objet d'affichage sous-jacent
    
    DisplayInfo() : id(-1), x(0), y(0), width(0), height(0), scale(1.0f), isPrimary(false), ptr(nullptr) {}
};

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    bool initialize();
    void updateDisplays();
    const std::vector<DisplayInfo>& getDisplays() const;
    const std::vector<DisplayInfo>& getAvailableDisplays() const { return getDisplays(); }
    const DisplayInfo* getPrimaryDisplay() const;
    const DisplayInfo* getDisplayForWindow(int windowX, int windowY) const;
    
    // Gestion du suivi de la souris
    void setFollowMouse(bool enable);
    bool isFollowingMouse() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
