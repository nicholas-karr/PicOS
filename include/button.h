#ifndef PICOS_BUTTON_H
#define PICOS_BUTTON_H

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
        //gpio_function f = gpio_get_function(port);

        bool ret = gpio_get(port); //(1ul << port) && gpio_get(port); //sio_hw->gpio_in;

        //gpio_set_function(port, f);

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

#endif