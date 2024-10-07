#include <array>
#include <pico/scanvideo.h>
#include <pico/scanvideo/composable_scanline.h>

#include "draw.h"
#include "screen.h"
#include "mem.h"

__not_in_flash("z") uint16_t tokBackground[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,
    COMPOSABLE_COLOR_RUN, COLOR_TRANSPARENT | PICO_SCANVIDEO_PIXEL_FROM_RGB5(1 << 3, 2, 2), SCREEN_WIDTH,
    COMPOSABLE_RAW_1P, 0
};

// Fragment that starts a line
// Inserts 3 transparent pixels
__not_in_flash("x") uint16_t tokTextLineBegin[] = {
    COMPOSABLE_COLOR_RUN, COLOR_HCAL, HCAL_DEFAULT,  // At least 3 pixels of black
    COMPOSABLE_RAW_RUN, COLOR_TRANSPARENT,
    SCREEN_WIDTH /* run length - 3 */, COLOR_TRANSPARENT, COLOR_TRANSPARENT
};

// Fragment that ends a line
__not_in_flash("y") uint16_t tokLineEnd[] = {
        COMPOSABLE_RAW_2P, 0,
        0, COMPOSABLE_RAW_1P_SKIP_ALIGN,
        0, 0,
        COMPOSABLE_EOL_SKIP_ALIGN, 0xffff
};

// A fragment of just transparent pixels
__not_in_flash("y") uint16_t tokTransparents[] = {
    COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT,
    COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT
};

constexpr std::array<uint8_t, 255> makeFontConvTable() {
    std::array<uint8_t, 255> fontConv = {};

    fontConv[' '] = 0;

    for (uint32_t i = 0; i <= 127; i++) {
        fontConv[i] = i - 0x20;
    }

    fontConv[0] = ' ' - 0x20;

    return fontConv;
}

// Converts between ASCII and offsets in the font sprites
__not_in_flash("z") std::array<uint8_t, 255> fontConv = makeFontConvTable();

void __time_critical_func(drawBackground) (uint16_t y, Layer background) {
    uint32_t* buf = background.data;
    memcpyFast<16>(buf, tokBackground); buf += 4;
    memcpyFast<16>(buf, tokLineEnd); buf += 4;
    background.data_used = (buf - background.data) / 2;
}

std::array<uint16_t*, 2> hcalInstances = {
    &tokBackground[2],
    &tokTextLineBegin[2],
};

uint16_t horizontalCalibration = HCAL_DEFAULT;

void setHorizontalCalibration(uint16_t hcal) {
    horizontalCalibration = hcal;

    for (uint16_t* i : hcalInstances) {
        *i = hcal;
    }
}

uint16_t getHorizontalCalibration() {
    return horizontalCalibration; 
}