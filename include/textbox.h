#ifndef PICOS_TEXTBOX_H
#define PICOS_TEXTBOX_H

#include <string_view>
#include <algorithm>
#include <vector>
#include <string>

#include "util.h"
#include "fontbuild.h"
#include "screen.h"
#include "draw.h"

#define STR_ALIGNED(S) ( (_Alignas(4) char []){ S } )

// Round up to the nearest FRAGMENT_SIZE
#define ROUND_UP(num) ((num + 3) & ~3)
//#define ROUND_DOWN(num, multiple) (num >= 0 ? (num / multiple) * multiple : ((num - multiple + 1) / multiple) * multiple)

struct __attribute__((__packed__)) TextLineEntry {
    // Text to be rendered
    const char* buf;

    // Pointer to last DMA instruction to be written by this entry
    const uint32_t* c_end;

    // Current line of the font bitmap to draw from
    uint32_t* font;
};

class Window {
public:
    enum class Type {
        TextBox,
        SnakeGame,
        SnakeScore
    };

    virtual std::string_view lineAt(int y) = 0;

    uint16_t x, x_max;
    uint16_t y, y_max;

    virtual Type getType() = 0;
};

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

        setAll(' ' - 0x20);
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

// List of all windows to be rendered, sorted by x position
static Window* windows[10];
static int windowCount = 0;

char altTextBuf[(SCREEN_WIDTH / FONT_WIDTH) * 2] = {};
char* altTextBufUsed = altTextBuf;

extern "C" void render_relevant_text_boxes(uint32_t* buf, void* unused, const TextLineEntry* relevant);


// Fill layer 0 with text
void __time_critical_func(renderTextBoxes) (uint16_t y, Layer text) {
    Window* relevant[windowCount];
    uint16_t relevantCount = 0;

    // Store which text boxes are on this scanline (probably has to happen every frame?)
    for (uint32_t i = 0; i < windowCount; ++i) {
        Window* test = windows[i];
        if (y >= test->y && y <= test->y_max) {
            relevant[relevantCount++] = test;
        }
    }

    // Pointer to an array of pointers to tokens/colors
    uint32_t* buf = text.data;

    *buf++ = ((uintptr_t)(tokTextLineBegin));

    TextLineEntry entries[8] = {};
    uint16_t entriesCnt = 0;
    uint16_t xitr = 0;
    uint32_t* bufItr = buf;

    for (uint16_t i = 0; i < relevantCount && entriesCnt < 8; ++i) {
        const Window& box = *relevant[i];

        if (box.x >= screen.x_end || xitr >= box.x_max) { 
            continue; 
        } // should be setting 'visible' flag too
        
        if (xitr < box.x) {
            // Pre-padding, one character at a time
            size_t length = (box.x - xitr) / 4;
            entries[entriesCnt++] = { (char*)nullptr, bufItr += length, nullptr }; // 4 pixels per 32-bit ptr
            xitr += length * 4;

            if (xitr != box.x) {
                CHECK(false);
            }
        }

        std::string_view line = relevant[i]->lineAt(y);
        if (line.size() != 0) {
            size_t length = line.size();
            const char* data = line.data();

            // Pad the end to 4 characters if it's not a multiple
            // todo: Pad correctly when followed by another window
            if (length % 4 != 0) {
                char* dataMod = altTextBufUsed;

                memset(dataMod, FONT_CHAR_TRANSPARENT, line.size() + 4);
                memcpy(dataMod, data, line.size());

                data = dataMod;
                length = ROUND_UP(length);
                altTextBufUsed += length + 4;
            }

            uint16_t localY = y - relevant[i]->y;

            // Try to remove some characters if another text box is over this one
            if (xitr > box.x) {
                length -= (xitr - box.x) / 4;
                length = ROUND_UP(length);
                size_t offset = ((xitr - box.x) / 4);
                offset = ROUND_UP(offset);
                if (offset >= line.size()) {
                    continue;
                }
                data += offset;
            }

            entries[entriesCnt++] = { data, bufItr += length, font_raw_pixels + FRAGMENT_WORDS * (localY % FONT_HEIGHT) }; // todo: replace aligned size with unaligned and use geq in asm
            xitr += length * 4;
        }
    }

    entries[entriesCnt++] = { nullptr, nullptr, nullptr }; // End of line meta token

    std::fill_n(bufItr, buf + 160 - bufItr, uintptr_t(tokTransparents));

    render_relevant_text_boxes(buf, nullptr, entries);
    
    buf += 160;

    *buf++ = ((uintptr_t)(tokLineEnd));
    *buf++ = 0;

    text.data_used = (buf - text.data) / 2;

    altTextBufUsed = altTextBuf;
}

#endif // ifndef PICOS_TEXTBOX_H