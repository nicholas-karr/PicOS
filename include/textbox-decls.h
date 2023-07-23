#include <pico/scanvideo.h>

//
// ITS LEGAL TO POINT TO LATER IN THE DMA BUFFER!!!!!!!
//

#define HCAL_DEFAULT 54 //todo: get rid of the ~10 pixels of right overscan (alignment issues :/)

extern __not_in_flash("x") uint16_t tokTextLineBegin[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,  // At least 3 pixels of black
    COMPOSABLE_RAW_RUN, 0,
    1280 /* run length - 3 */, 0, 0 // It's 1280 because math sorry
};

extern __not_in_flash("y") uint16_t tokTextLineEnd[] = {
#if FRAGMENT_WORDS == 5 || FRAGMENT_WORDS == 3
        COMPOSABLE_RAW_1P, 0,
#endif
#if FRAGMENT_WORDS == 3
        COMPOSABLE_RAW_1P, 0,
#endif
#if FRAGMENT_WORDS >= 4
        COMPOSABLE_RAW_2P, 0,
        0, COMPOSABLE_RAW_1P_SKIP_ALIGN,
        0, 0,
#endif
        COMPOSABLE_EOL_SKIP_ALIGN, 0xffff
};

extern __not_in_flash("y") uint16_t tokSkipLine[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,
    COMPOSABLE_COLOR_RUN, 0,
    1280 /* run length - 3 */, // It's 1280 because math sorry
    COMPOSABLE_RAW_1P, 0xffff
};

extern __not_in_flash("y") uint16_t tokSkipLine2[] = {
    COMPOSABLE_RAW_1P, 0xffff,
    COMPOSABLE_RAW_1P, 0xffff,
    COMPOSABLE_RAW_1P, 0xffff,
    COMPOSABLE_EOL_SKIP_ALIGN, 0xffff,
};

// A fragment of just transparent pixels
extern __not_in_flash("y") uint16_t tokTransparents[] = {
    //1 << 3, 1 << 3, 1 << 3, 1 << 3,
    //1 << 3, 1 << 3, 1 << 3, 1 << 3,
    //1, 1, 1, 1, 1, 1, 1, 1
    0,0,0,0, 0,0,0,0

    //PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK,
    //PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK, PICO_SCANVIDEO_ALPHA_MASK
};



extern __not_in_flash("z") uint16_t instantTerminateLine[] = {
    // Horizontal calibration
    COMPOSABLE_COLOR_RUN, 0b10000, 200 /* hcal */,  // At least 3 pixels of black

    //COMPOSABLE_COLOR_RUN, 0b1111111111111110, 1280 - HCAL_DEFAULT /* hcal */,
    //COMPOSABLE_COLOR_RUN, 0b1111111111111110, 1280 - HCAL_DEFAULT /* hcal */,
    COMPOSABLE_COLOR_RUN, PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(2, 2, 2), 800,       //1280 - HCAL_DEFAULT /* hcal */,

    COMPOSABLE_RAW_1P, 1
};

// Make ASCII into font offsets
extern __not_in_flash("z") char fontConv[255] = {};

void makeFontConvTable() {
    fontConv[' '] = 0;

    for (uint32_t i = 'a'; i <= 'z'; i++) {
        fontConv[i] = i - 0x20;
    }

    for (uint32_t i = 'A'; i <= 'Z'; i++) {
        fontConv[i] = i - 0x20;
    }

    
}