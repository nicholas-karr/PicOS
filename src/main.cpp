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
#include <cmath>

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

void __time_critical_func(core1_vga_main)() {
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    gpio_deinit(0); // Reduce banding on my board

    while (true) {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(false);
        if (scanline_buffer != nullptr) {
            uint16_t y = scanvideo_scanline_number(scanline_buffer->scanline_id);
            uint16_t frame = scanvideo_frame_number(scanline_buffer->scanline_id);

            renderTextBoxes(scanline_buffer, y);

            scanvideo_end_scanline_generation(scanline_buffer);
        }
    }
}



int __time_critical_func(core0_vga_main)() {
    set_sys_clock_khz(148500, true); // 720p //todo: * 3/2
    //set_sys_clock_khz(74250, true); // 720p
    //set_sys_clock_khz(120000, true); // vga_mode_1280x1024_40
    //setup_default_uart();
    stdio_init_all();

    screen.init(vga_mode);

    //setWholeScreenDocument();

    build_font();

    TextBox& spinbox1 = textBoxes[1];
    TextBox& spinbox2 = textBoxes[0];
    textBoxesCount = 2;

    spinbox1.init(40, 200, 0, 200, "CONTENTS OF THE FIRST\nNEWLINE", false);
    spinbox2.init(300, 400, 100, 300, "CONTENTS OF THE SECOND\nNEWLINE\n\nHELLO", false);

    multicore_launch_core1(core1_vga_main);

    


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


            double ms = double(time_us_64()) / 1'000'000'000.0;
            //spinbox1.x = (cos(ms) + 1) * 200;
            //spinbox1.y = (sin(ms) + 1) * 300;
            //spinbox2.x = (cos(ms + M_PI_2) + 1) * 200;
            //spinbox2.y = (sin(ms + M_PI_2) + 1) * 300;

            int c = getchar();
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
    //auto i = offsetof(TextBox, cursor);

    core0_vga_main();
}