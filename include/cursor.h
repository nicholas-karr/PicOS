#ifndef PICOS_CURSOR_H
#define PICOS_CURSOR_H

#include "screen.h"
#include "textbox.h"

struct TextCursor {
    constexpr static const char CHAR_SELECT = '\x7F' - 0x20;

    TextLine* line = nullptr;
    uint16_t pos = 0;
    char orig = '\0';

    void set() {
        orig = line->data()[pos];
        line->data()[pos] = CHAR_SELECT;
    }

    void unset() {
        line->data()[pos] = orig;
    }

    bool operator==(const TextCursor& lhs) const {
        return line == lhs.line && orig == lhs.orig;
    }
};

class Cursor {
    public:
    uint16_t x_;
    uint16_t y_;

    TextCursor sel;

    void move(uint16_t x, uint16_t y) {
        x_ = x;
        y_ = y;

        return;

        for (uint16_t i = 0; i < windowCount; ++i) {
            Window* window = windows[i];
            TextBox* test = nullptr;

            if (window->getType() == Window::Type::TextBox) { test = (TextBox*)window; }
            else { continue; }

            if (y_ >= test->y && y_ <= test->y_max && x_ >= test->x && x_ <= test->x_max) {
                auto line = test->editableLineAt(y_);
                if (!line) {
                    return;
                }
                uint16_t pos = x_ - test->x;
                pos /= 4;

                if (pos >= line->allocLen_) {
                    line->reserve(test->getWidth() / 4);
                }

                TextCursor next = { line, pos };

                if (next != sel) {
                    sel.unset();
                    sel = next;
                    sel.set();
                }

                return;
            }
        }
    }
};

Cursor mouse;

#endif