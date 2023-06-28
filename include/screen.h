#include "pico/scanvideo.h"

class Screen {
public:
    int width;
    int height;

    int x_start;
    int x_end;

    int y_start;
    int y_end;

    void init(const scanvideo_mode_t& mode) {
        width = x_end = mode.width;
        height = y_end = mode.height;

        x_start = y_start = 0;
    }
};

static Screen screen;