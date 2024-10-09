#include <menu.h>

CONSOLE_STATE consoleState = CONSOLE_STATE::MAIN_MENU;

Mode* modes[8];
int modeCount;

Window* mainMenu;
Window* optionsMenu;

void bindMenus() {
    auto main = new Menu<3, 60>(100, 100);

    main->items[0] = { "Raspberry Pi Pico Computer", nullptr };
    main->items[1] = { "By Nicholas Karr", [](){} };
    main->items[2] = { "Play Snake", [](){} };
        
    main->defaultSelection = 2;

    mainMenu = (Window*)main;
}