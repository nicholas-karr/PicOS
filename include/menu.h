#ifndef PICOS_MENU_H
#define PICOS_MENU_H

#include <window.h>

enum CONSOLE_STATE {
    MAIN_MENU,
    OPTIONS,
    OPTIONS_HCAL,
    OPTIONS_VCAL,
    GAME_SNAKE
};

extern CONSOLE_STATE consoleState;

using VoidFn = void(*)();

struct MenuItem {
    std::string_view text;

    // nullptr if not selectable
    VoidFn onSelect;
};

// A type of Window that presents an ordered list and lets a user interact with it
template<int LINE_COUNT, int LINE_LENGTH_MAX>
class Menu : FixedTextWindow<LINE_LENGTH_MAX, LINE_COUNT> {
public:
    std::array<MenuItem, LINE_COUNT> items;
    int defaultSelection;

    Menu(int x, int y) : FixedTextWindow<LINE_LENGTH_MAX, LINE_COUNT>(x, y) {}
};

// A mode is a bank of windows that are rendered at the same time
class Mode {
    Window* windows[8];
    int windowCount;
};

extern Mode* modes[8];
extern int modeCount;

constexpr static std::string_view MENU_CONTROL_GUIDE = "BACK  OK  NEXT";

void bindMenus();

extern Window* mainMenu;
extern Window* optionsMenu;

#endif // ifndef PICOS_MENU_H