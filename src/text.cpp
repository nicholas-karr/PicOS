#include <text.h>
#include <font.h>
#include <window.h>

#include <array>
#include <cstring>

constexpr std::array<uint8_t, 255> makeFontConvTable() {
    std::array<uint8_t, 255> fontConv = {};

    for (uint32_t i = 0x20; i <= 0x20 + 96; i++) {
        fontConv[i] = i - 0x20;
    }

    fontConv[0] = 0;

    return fontConv;
}

// Converts between ASCII and offsets in the font sprites
__not_in_flash("z") std::array<uint8_t, 255> fontConv = makeFontConvTable();

struct __attribute__((__packed__)) TextLineEntry {
    // Text to be rendered
    const char* buf;

    // Pointer to last DMA instruction to be written by this entry
    const uint32_t* c_end;

    // Current line of the font bitmap to draw from
    uint32_t* font;
};

char altTextBuf[(SCREEN_WIDTH / FONT_WIDTH) * 2] = {};
char* altTextBufUsed = altTextBuf;

extern "C" void render_relevant_text_boxes(uint32_t* buf, void* unused, const TextLineEntry* relevant);

// Render the list of windows on a scanline
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