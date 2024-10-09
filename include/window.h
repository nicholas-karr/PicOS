#ifndef PICOS_WINDOW_H
#define PICOS_WINDOW_H

#include <screen.h>
#include <text.h>

// A window stored as a text buffer of fixed size that may be modified
template <int WIDTH, int HEIGHT>
class FixedTextWindow : public Window {
public:
    using ThisT = FixedTextWindow<WIDTH, HEIGHT>;

    //static_assert(WIDTH % FRAGMENT_WORDS == 0);

    char text[WIDTH * HEIGHT] = {};

    FixedTextWindow(int x, int y) {
        this->x = x;
        this->y = y;

        this->x_max = x + WIDTH * FONT_WIDTH;
        this->y_max = y + HEIGHT * FONT_HEIGHT - 1;

        setAll(fontConv[' ']);
    }

    // Access the text character at a position on the field
    char& at(Position pos) {
        CHECK(pos.x >= 0 && pos.x <= WIDTH && pos.y >= 0 && pos.y <= HEIGHT);

        return text[(pos.y * WIDTH) + pos.x];
    }

    void setAll(char c) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                at({x, y}) = c;
            }
        }
    }

    std::string_view lineAt(int y) {
        y -= this->y;
        int index = y / FONT_HEIGHT;
        if (index < 0 || index > HEIGHT + 1) {
            return nullptr;
        }
        
        return std::string_view(text + index * (WIDTH), WIDTH);
    }

    void convAsciiToRender() {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                char& c = at({x, y});
                c = fontConv[c];
            }
        }
    }

    void copyFrom(const char other[WIDTH * HEIGHT]) {
        //static_assert((WIDTH * HEIGHT) % 4 == 0);

        memcpyFast<WIDTH * HEIGHT>(text, other);
    }
};

#endif // ifndef PICOS_WINDOW_H