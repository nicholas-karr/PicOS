#include <atomic>
#include <optional>
#include <stdint.h>
#include <string.h>
#include <cmath>

#include "pico.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/structs/sio.h"
#include "hardware/structs/padsbank0.h"
#include "hardware/structs/iobank0.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#include "mem.h"
#include "input.h"
#include "draw.h"
#include "snake.h"
#include "menu.h"

#define vgaMode vga_mode_720p_60

std::atomic<uint16_t> frameNum;

void __time_critical_func(core1_vga_main)() {
    scanvideo_setup(&vgaMode);
    scanvideo_timing_enable(true);

    gpio_deinit(0); // Reduce banding on my board
    gpio_deinit(buttons[1].port_); // Reduce banding on my board
    gpio_deinit(buttons[2].port_); // Reduce banding on my board

    while (true) {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(true);

        if (scanline_buffer != nullptr) {
            uint16_t y = scanvideo_scanline_number(scanline_buffer->scanline_id);
            frameNum = scanvideo_frame_number(scanline_buffer->scanline_id);

            // Manual interlacing: It kind of works!
            /*if ((y % 2) == (frameNum % 2)) {
                scanline_buffer->status = SCANLINE_SKIPPED;
                scanvideo_end_scanline_generation(scanline_buffer);
                continue;
            }*/

            // Not DMA
            drawBackground(y, {scanline_buffer->data, scanline_buffer->data_used});

            // Uses fixed fragment DMA for text rendering
            scanline_buffer->fragment_words2 = FRAGMENT_WORDS;
            renderTextBoxes(y, {scanline_buffer->data2, scanline_buffer->data2_used});

            scanline_buffer->status = SCANLINE_OK;
            scanvideo_end_scanline_generation(scanline_buffer);
        }
    }
}



int __time_critical_func(core0_vga_main)() {
    screen.init(vgaMode);

    build_font();

    initSnakeGame();
    windows[windowCount++] = (Window*)snakeInst;
    windows[windowCount++] = (Window*)snakeScoreInst;

    multicore_launch_core1(core1_vga_main);

    buttons[0].init();
    buttons[1].init();
    buttons[2].init();

    uint16_t nextX = snakeScoreInst->x;
    uint16_t nextXMax = snakeScoreInst->x_max;

    while (true) {
        if (scanvideo_in_vblank()) {
            uint32_t frameNumLoad = frameNum.load();

            //snakeScoreInst->x = nextX;
            //snakeScoreInst->x_max = nextXMax;

            snakeInst->tick(frameNumLoad);
            snakeScoreInst->tick(frameNumLoad);

            while (frameNumLoad == frameNum.load()) {
                sleep_us(100);
            }
        }

        int hsize = snakeScoreInst->x_max - snakeScoreInst->x;
        nextX = 250.0 + 150.0 * cosf(float(frameNum) / 50.0);
        nextX = ROUND_UP(nextX);
        nextXMax = nextX + hsize;
    }
}

int main() {
    // Overclock to the required rate for 720p60 VGA
    set_sys_clock_khz(148500, true);

    // Initialize USB serial
    stdio_init_all();

    bindMenus();

    core0_vga_main();
}