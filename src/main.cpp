#include <atomic>
#include <optional>

#include <stdint.h>
#include <string.h>
#include "pico.h"
#include "hardware/gpio.h"
//#include "spans.h"
#include "hardware/clocks.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "pico/stdlib.h"
#include <cmath>

#include "pico.h"
#include "hardware/structs/sio.h"
#include "hardware/structs/padsbank0.h"
#include "hardware/structs/iobank0.h"
#include "hardware/irq.h"

#include <cstdio>

#include "mem.h"
#include "button.h"
#include "textbox.h"
#include "input.h"
#include "snake.h"

#define vga_mode vga_mode_720p_60 

std::atomic<uint16_t> frameNum;

extern __not_in_flash("z") uint16_t tokBackground[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,
    COMPOSABLE_COLOR_RUN, PICO_SCANVIDEO_ALPHA_MASK | PICO_SCANVIDEO_PIXEL_FROM_RGB5(1 << 3, 2, 2), 1280,
    COMPOSABLE_RAW_1P, 0
};

extern __not_in_flash("z") uint16_t tokNone[] = {
    COMPOSABLE_COLOR_RUN, 0, HCAL_DEFAULT /* hcal */,
    COMPOSABLE_COLOR_RUN, 0xFFFF, 1280,
    COMPOSABLE_RAW_1P, 0
};

void __time_critical_func(drawBackground) (uint16_t y, Layer background) {
    uint32_t* buf = background.data;
    memcpyFast<16>(buf, tokBackground); buf += 4;
    memcpyFast<16>(buf, tokTextLineEnd); buf += 4;
    background.data_used = (buf - background.data) / 2;
}

void __time_critical_func(drawIcons) (uint16_t y, Layer layer) {
    uint32_t* buf = layer.data;
    //memcpy(buf, tokNone, sizeof(tokNone)); buf += 4;
    memcpy(buf, tokTextLineEnd, sizeof(tokTextLineEnd)); buf += 4;
    layer.data_used = (buf - layer.data) / 2;
}

void __time_critical_func(core1_vga_main)() {
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    gpio_deinit(0); // Reduce banding on my board
    gpio_deinit(buttons[1].port_); // Reduce banding on my board
    gpio_deinit(buttons[2].port_); // Reduce banding on my board

    while (true) {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(false);

        if (scanline_buffer != nullptr) {
            uint16_t y = scanvideo_scanline_number(scanline_buffer->scanline_id);
            frameNum = scanvideo_frame_number(scanline_buffer->scanline_id);

            // NOT DMA
            //scanline_buffer->fragment_words = FRAGMENT_WORDS;
            drawBackground(y, {scanline_buffer->data, scanline_buffer->data_used});

            scanline_buffer->fragment_words2 = FRAGMENT_WORDS;
            renderTextBoxes(y, {scanline_buffer->data2, scanline_buffer->data2_used});

            //scanline_buffer->fragment_words3 = FRAGMENT_WORDS;
            //drawIcons(y, {scanline_buffer->data2, scanline_buffer->data2_used});

            scanline_buffer->status = SCANLINE_OK;
            scanvideo_end_scanline_generation(scanline_buffer);
        }

        //if (frameNum.load() % 10 == 0) {
            //buttons[0].update();
            //buttons[1].update();
            //buttons[2].update();
        //}
    }
}



int __time_critical_func(core0_vga_main)() {
    screen.init(vga_mode);

    build_font();
    makeFontConvTable();
    loadPadEnd();

    TextBox* spinbox1 = new TextBox();
    spinbox1->init(0, 22 * 4, 0, 22 * FONT_HEIGHT, "Contents of \x7Fthe\nNEWLINE\n", true);
    windows[1] = (Window*)spinbox1;
    //windowCount++;

    initSnakeGame();
    windows[windowCount++] = (Window*)snakeInst;

    multicore_launch_core1(core1_vga_main);

    buttons[0].init();
    buttons[1].init();
    buttons[2].init();

    while (true) {
        

    
        if (scanvideo_in_vblank()) {
            
            //uint16_t& hcal = beginning_of_line[2];
            //if (buttons[0].get()) {
                //color = ((color << 1) == 0) ? 1 : color << 1; //rollover
            //    screen.x_start++;
            //}
            //if (buttons[2].get()) {
            //    screen.x_start--;
            //}


            serial.run();

            snakeInst->tick(frameNum.load());

            //spinbox1.x = ROUND_UP(inputState.mouse_x, 4);
            //spinbox1.x_max = spinbox1.x + 80;

            //spinbox1.y = ROUND_UP(inputState.mouse_y, 4);
            //spinbox1.y_max = spinbox1.y + 80;

            /*spinbox1.x = 250.0 + 150.0 * cosf(float(frameNum) / 50.0);
            spinbox1.x = ROUND_UP(spinbox1.x, 4);
            spinbox1.x_max = spinbox1.x + 80;

            spinbox1.y = 250.0 + 150.0 * sinf(float(frameNum) / 50.0);
            spinbox1.y = ROUND_UP(spinbox1.y, 4);
            spinbox1.y_max = spinbox1.y + 160;*/

            /*spinbox2.x = 250.0 + 150.0 * cosf(float(frameNum) / 50.0 + M_PI);
            spinbox2.x = ROUND_UP(spinbox2.x, 4);
            spinbox2.x_max = spinbox2.x + 80;

            spinbox2.y = 250.0 + 150.0 * sinf(float(frameNum) / 50.0 + M_PI);
            spinbox2.y = ROUND_UP(spinbox2.y, 4);
            spinbox2.y_max = spinbox2.y + 160;*/

            int c = getchar_timeout_us(0);
            //int c = EOF;
            if (c == EOF) {}
            else if (c == 'r') {
                //screen.x_start--;
                tokTextLineBegin[2]++;
                printf("to %d\r\n", tokTextLineBegin[2]);
            }
            else if (c == 'l') {
                tokTextLineBegin[2]--;
                printf("to %d\r\n", tokTextLineBegin[2]);
            }
        }
    }
}


int main() {
    set_sys_clock_khz(148500, true); // 720p //todo: * 3/2

    stdio_init_all();

    core0_vga_main();
}