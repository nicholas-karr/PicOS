#ifndef PICOS_BUTTON_H
#define PICOS_BUTTON_H

#include <array>
#include "hardware/gpio.h"

class Button {
public:
    uint32_t port_;
    uint8_t history_ = 0;
    bool status;
    bool justPressed_ = false;

    void init() {
        gpio_deinit(port_);
        gpio_init(port_);
        gpio_set_dir(port_, GPIO_IN);
    }

    inline static bool cleanGet(uint32_t port) {
        bool ret = gpio_get(port);

        return ret;
    }

public:
    Button(uint32_t port) : port_(port) {}

    void update() {
        history_ <<= 1;
        history_ |= cleanGet(port_);

        if (history_ == 0b01111111) {
            justPressed_ = true;
        }
    }

    // A button has been pressed when it was read as pressed 8 times
    // This eliminates bouncing
    bool get() {
        return history_ == 255;
    }

    bool justPressed() {
        if (justPressed_) {
            justPressed_ = false;
            return true;
        }
        return false;
    }
};

static std::array<Button, 3> buttons = { Button(0), Button(6), Button(11) };

#endif