#include "pico/scanvideo.h"

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

static Screen screen;