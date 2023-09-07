#ifndef PICOS_DEVICE_H
#define PICOS_DEVICE_H

#include "button.h"

// Global device state: buttons and rendering

Button buttons[3] = { Button(0), Button(6), Button(11) };

struct VideoProfile_720p60fps {
    static constexpr scanvideo_timing_t timing = {
                .clock_freq = 74250000,

                .h_active = 1280,
                .v_active = 720,

                .h_front_porch = 110,
                .h_pulse = 40,
                .h_total = 1650,
                .h_sync_polarity = 1,

                .v_front_porch = 5,
                .v_pulse = 5,
                .v_total = 750,
                .v_sync_polarity = 1,

                .enable_clock = 0,
                .clock_polarity = 0,

                .enable_den = 0
        };
    
    static constexpr scanvideo_mode_t mode = 
        {
                .default_timing = &timing,
                .pio_program = &video_24mhz_composable,
                .width = 1280,
                .height = 720,
                .xscale = 1,
                .yscale = 1,
        };
};

#endif