#include <pico/scanvideo.h>

#define HCAL_DEFAULT 50 //todo: get rid of the ~10 pixels of right overscan (alignment issues :/)

extern __not_in_flash("x") uint16_t tokTextLineBegin[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,  // At least 3 pixels of black
    COMPOSABLE_RAW_RUN, 0,                  // 
    1280 /* run length - 3 */, 0, 0
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
        COMPOSABLE_EOL_SKIP_ALIGN, 0xffff // eye catcher
};

// A fragment of just transparent pixels
extern __not_in_flash("y") uint16_t tokTransparents[] = {
    1 << 3, 1 << 3, 1 << 3, 1 << 3,
    1 << 3, 1 << 3, 1 << 3, 1 << 3,
};