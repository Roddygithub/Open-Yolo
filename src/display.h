#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>

struct Screen {
    int width;
    int height;
    float dpi;
};

class DisplayManager {
public:
    void init();
    bool isRunning();
    void swapBuffers();
    void cleanup();
    std::vector<Screen> getScreens();
};

#endif // DISPLAY_H

