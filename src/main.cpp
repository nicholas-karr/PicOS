#include <atomic>
#include <optional>

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
#include "input.h"

#define FRAGMENT_WORDS 4

// vga_mode_1280x1024_40 
//#define vga_mode vga_mode_640x480_60
#define vga_mode vga_mode_720p_60 

Button buttons[3] = { Button(0), Button(6), Button(11) };

std::atomic<uint16_t> frameNum;

void __time_critical_func(core1_vga_main)() {
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);

    gpio_deinit(0); // Reduce banding on my board

    while (true) {
        scanvideo_scanline_buffer* scanline_buffer = scanvideo_begin_scanline_generation(false);
        if (scanline_buffer != nullptr) {
            uint16_t y = scanvideo_scanline_number(scanline_buffer->scanline_id);
            frameNum = scanvideo_frame_number(scanline_buffer->scanline_id);

            scanline_buffer->fragment_words = FRAGMENT_WORDS;
            renderTextBoxes(y, scanline_buffer->data, scanline_buffer->data_used);

            //uint32_t* buf = scanline_buffer->data;
            //*buf++ = ((uintptr_t)(instantTerminateLine));
            //*buf++ = 4;
            //*buf++ = ((uintptr_t)(tokTextLineEnd));
            //*buf++ = 4;
            //scanline_buffer->data_used = buf - scanline_buffer->data;

            //uint32_t* buf2 = scanline_buffer->data2;
            //*buf2++ = ((uintptr_t)(instantTerminateLine2));
            //*buf2++ = ((uintptr_t)(tokTextLineEnd));
            //*buf2++ = 0;
            //scanline_buffer->data2_used = buf2 - scanline_buffer->data2;

            //std::swap(scanline_buffer->data, scanline_buffer->data2);
            //std::swap(scanline_buffer->data_used, scanline_buffer->data2_used);
            

            //instantTerminateLine[4] = 1;


            //scanline_buffer->data = scanline_buffer->data2;
            //scanline_buffer->data_used = scanline_buffer->data2_used;

            scanline_buffer->status = SCANLINE_OK;
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

    TextBox& spinbox1 = textBoxes[0];
    TextBox& spinbox2 = textBoxes[1];
    textBoxesCount = 2;

    spinbox1.init(40, 200, 0, 200, "CONTENTS OF THE FIRST\nNEWLINE", true);
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


            serial.run();

            spinbox1.x = ROUND_UP(inputState.mouse_x, 4);
            spinbox1.x_max = spinbox1.x + 80;

            spinbox1.y = ROUND_UP(inputState.mouse_y, 4);
            spinbox1.y_max = spinbox1.y + 80;

            /*spinbox1.x = 250.0 + 150.0 * cosf(float(frameNum) / 50.0);
            spinbox1.x = ROUND_UP(spinbox1.x, 4);
            spinbox1.x_max = spinbox1.x + 80;

            spinbox1.y = 250.0 + 150.0 * sinf(float(frameNum) / 50.0);
            spinbox1.y = ROUND_UP(spinbox1.y, 4);
            spinbox1.y_max = spinbox1.y + 160;*/

            spinbox2.x = 250.0 + 150.0 * cosf(float(frameNum) / 50.0 + M_PI);
            spinbox2.x = ROUND_UP(spinbox2.x, 4);
            spinbox2.x_max = spinbox2.x + 80;

            spinbox2.y = 250.0 + 150.0 * sinf(float(frameNum) / 50.0 + M_PI);
            spinbox2.y = ROUND_UP(spinbox2.y, 4);
            spinbox2.y_max = spinbox2.y + 160;

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
    serial.init();

    core0_vga_main();
}