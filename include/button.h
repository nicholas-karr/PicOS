#include "hardware/gpio.h"

class Button {
private:
    uint32_t port_;
    uint8_t history_ = 0;

    inline static bool cleanGet(uint32_t port) {
        gpio_function f = gpio_get_function(port);

        gpio_deinit(port);
        gpio_init(port);

        bool ret = (1ul << port) && sio_hw->gpio_in;

        gpio_set_function(port, f);

        return ret;
    }

public:
    Button(uint32_t port) : port_(port) {}

    bool get() {
        history_ <<= 1;
        history_ |= cleanGet(port_);

        return (history_ == 0b01111111);
    }
};