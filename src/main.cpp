#include <stdint.h>
#include <string.h>
#include "pico.h"
#include "hardware/gpio.h"
#include "spans.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "pico/stdlib.h"

#include "pico.h"
#include "hardware/structs/sio.h"
#include "hardware/structs/padsbank0.h"
#include "hardware/structs/iobank0.h"
#include "hardware/irq.h"

#include <cstdio>

#include "button.h"
#include "textbox.h"

#define FRAGMENT_WORDS 4

// vga_mode_1280x1024_40 
//#define vga_mode vga_mode_640x480_60
#define vga_mode vga_mode_720p_60 




Button buttons[3] = { Button(0), Button(6), Button(11) };






#if PICO_SCANVIDEO_PLANE1_FRAGMENT_DMA
static __not_in_flash("x") uint16_t beginning_of_line[] = {
        // todo we need to be able to shift scanline to absorb these extra pixels
#if FRAGMENT_WORDS == 5
        COMPOSABLE_RAW_1P, 0,
#endif
#if FRAGMENT_WORDS >= 4
        //COMPOSABLE_RAW_1P, 0,
#endif
        //COMPOSABLE_RAW_1P, 0,

        // Color run for hcal
        //COMPOSABLE_RAW_1P, 0,
        //COMPOSABLE_RAW_1P, 0,
        COMPOSABLE_COLOR_RUN, 0, 100, 

        // main run, 2 more black pixels
        COMPOSABLE_RAW_RUN, 0,


        0, // COUNT * 2 * FRAGMENT_WORDS - 3 + 2;
        0, 0

        #if 0
        0/*COUNT * 2 * FRAGMENT_WORDS -3 + 2*/, 0
        #endif
};
static __not_in_flash("y") uint16_t end_of_line[] = {
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
#endif
        


int32_t single_color_scanline(uint32_t *buf, size_t buf_length, int width, uint32_t color16) {
    assert(buf_length >= 2);

    //assert(width >= MIN_COLOR_RUN);
    // | jmp color_run | color | count-3 |  buf[0] =
    buf[0] = COMPOSABLE_COLOR_RUN | (color16 << 16);
    buf[1] = (width - 3) | (COMPOSABLE_RAW_1P << 16);
    // note we must end with a black pixel
    buf[2] = 0 | (COMPOSABLE_EOL_ALIGN << 16);

    return 3;
}
int chStart = 10;

void renderScanline (scanvideo_scanline_buffer* dest) {
    //constexpr const uint32_t COUNT = 10;
    //constexpr const uint32_t FRAGMENT_WORDS = 3;

    #define COUNT 74

    uint32_t* buf2 = dest->data;
    uint32_t* buf = buf2;
    size_t bufLen = dest->data_max;

    int y = scanvideo_scanline_number(dest->scanline_id);

    dest->fragment_words = FRAGMENT_WORDS;

    beginning_of_line[5] = COUNT * 2 * FRAGMENT_WORDS - 3 + 2   + 1;

    *buf++ = ((uintptr_t)(beginning_of_line));

    uint32_t *dbase = font_raw_pixels + FONT_WIDTH_WORDS * (y % (font->line_height));
    int cmax = font->dsc->cmaps[0].range_length;
    int ch = chStart; //(y / font->line_height) % cmax;

    for (int i = 0; i < COUNT; i++) {
        ch++;
        if (ch == cmax) ch = 1;

        *buf++ = ((uintptr_t)(dbase + ch * FONT_HEIGHT * FONT_WIDTH_WORDS));
    }

    *buf++ = ((uintptr_t)(end_of_line));
    *buf++ = 0;

    dest->data_used = (uint16_t) (buf - buf2);
    dest->status = SCANLINE_OK;
}

int __time_critical_func(vga_main)() {
    set_sys_clock_khz(148500, true); // 720p
    //set_sys_clock_khz(74250, true); // 720p
    //set_sys_clock_khz(120000, true); // vga_mode_1280x1024_40
    //setup_default_uart();
    stdio_init_all();

    screen.init(vga_mode);
    //screen.y_end /= 2; //todo: split line (lol this halves it horizontally)

    setWholeScreenDocument();

    build_font();

    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    gpio_deinit(0); // Reduce banding

    //gpio_deinit(0);
    //gpio_deinit(6);
    //gpio_deinit(11);

    uint16_t color = 1 << 0;

    while (true) {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);
        //if (scanline_buffer != nullptr) {
            int y = scanvideo_scanline_number(scanline_buffer->scanline_id);
            //int frame = scanvideo_frame_number(scanline_buffer->scanline_id);

            renderTextBoxes(scanline_buffer, y);
            //renderScanline(scanline_buffer);

            scanvideo_end_scanline_generation(scanline_buffer);
        //}
        /*else if (scanvideo_in_vblank()) {
            //uint16_t& hcal = beginning_of_line[2];
            if (buttons[0].get()) {
                //color = ((color << 1) == 0) ? 1 : color << 1; //rollover
                screen.x_start++;
            }
            if (buttons[2].get()) {
                screen.x_start--;
            }

            // Reset text box cursors
            resetTextBoxes();
        }*/
    }
}

int main() {

    vga_main();

    
}