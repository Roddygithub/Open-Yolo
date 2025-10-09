#ifndef GUI_H
#define GUI_H

#include "cursor_manager.h"

class GUI {
    CursorManager* cursorManager;

public:
    GUI(CursorManager* cm);
    void show();
    void update();
};

#endif  // GUI_H
