#ifndef PICOS_SCREEN_H
#define PICOS_SCREEN_H

#include <string_view>

#include "pico/scanvideo.h"

#include "font.h"
#include "draw.h"
#include "util.h"

class Screen {
public:
    uint16_t width;
    uint16_t height;

    uint16_t x_start;
    uint16_t x_end;

    uint16_t y_start;
    uint16_t y_end;

    void init(const scanvideo_mode_t& mode) {
        width = x_end = mode.width;
        height = y_end = mode.height;

        x_start = y_start = 0;
    }
};

extern Screen screen;

class Window {
public:
    virtual std::string_view lineAt(int y) = 0;

    uint16_t x, x_max;
    uint16_t y, y_max;
};

// List of all windows to be rendered, sorted by x position
extern Window* windows[10];
extern int windowCount;

#endif