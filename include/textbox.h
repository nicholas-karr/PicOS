#include "fontbuild.h"
#include "screen.h"
#include "ringbuf.h"

static __not_in_flash("x") uint16_t tokTextLineBegin[] = {
    COMPOSABLE_COLOR_RUN, 0, 0 /* hcal */,  // At least 3 pixels of black
    COMPOSABLE_RAW_RUN, 0,                  // 
    1280 /* run length - 3 */, 0, 0
};

static __not_in_flash("y") uint16_t tokTextLineEnd[] = {
#if FRAGMENT_WORDS == 5 || FRAGMENT_WORDS == 3
        COMPOSABLE_RAW_1P, 0,
#endif
#if FRAGMENT_WORDS == 3
        COMPOSABLE_RAW_1P, 0,
#endif
#if FRAGMENT_WORDS >= 4
        COMPOSABLE_RAW_2P, 0,
        0, COMPOSABLE_RAW_1P_SKIP_ALIGN,
        0, 0,
#endif
        COMPOSABLE_EOL_SKIP_ALIGN, 0xffff // eye catcher
};

// A fragment of just transparent pixels
static __not_in_flash("y") uint16_t tokTransparents[] = {
    1, 1, 1, 1
};

struct TextLinePreamble {
    uint16_t buf[8];

    void setFollowingSize(int size) {
        buf[5] = size;
    }

    void setHcal(int hcal) {
        buf[2] = hcal;
    }

    TextLinePreamble() {
        memcpy(buf, tokTextLineBegin, sizeof(buf));
    }
};

class TextBox {
public:
    uint16_t x, x_max;
    uint16_t y, y_max;

    char* cursor;
    char text[80];

    void initWholeScreen() {
        x = y = 0;
        x_max = screen.x_end;
        y_max = screen.y_end;
        cursor = text;
    }
    
    void onFrame(int frame) {
        cursor = text;
    }

    void renderScanline() {
        
    }

    int getWidth() { return x_max - x; }
    int getHeight() { return y_max - y; }
};

static TextBox textBoxes[10];
static int textBoxesCount = 0;

static RingBuffer<TextLinePreamble, 32> preambles;

uint32_t* getPreamble(int length) {
    TextLinePreamble* i = preambles.alloc();
    i->setFollowingSize(length);
    return (uint32_t*)(i->buf);

    //return (uint32_t*)(preambles.buf[0].buf);
}


void resetTextBoxes() {
    for (int i = 0; i < textBoxesCount; i++) {
        textBoxes[i].cursor = textBoxes[i].text;
    }
}

// Fill layer 0 with text
void renderTextBoxes(scanvideo_scanline_buffer* dest, int y) {
    constexpr int expectedWidth = 160; //(1280 / (8 * 2));

    TextBox* relevant[textBoxesCount];
    int relevantCount = 0;

    // Store which text boxes are on this scanline
    for (int i = 0; i < textBoxesCount; ++i) {
        TextBox& test = textBoxes[i];
        if (y >= test.y & y <= test.y_max) {
            relevant[relevantCount++] = &test;
        }
    }


    // Pointer to an array of pointers to tokens/colors
    uint32_t* buf = dest->data;

    *buf++ = ((uintptr_t)(tokTextLineBegin));
    //*buf++ = ((uintptr_t)(getPreamble(expectedWidth * 2 * FRAGMENT_WORDS)));

    uint32_t* dbase = font_raw_pixels + FONT_WIDTH_WORDS * (y % (font->line_height));

    //for (int x = screen.x_start; x < screen.x_end; x += 6) {
    for (int x = 0; x < expectedWidth; x += 1) {
        //todo: align

        //*buf++ = ((uintptr_t)(dbase + 'A' * FONT_HEIGHT * FONT_WIDTH_WORDS));

        TextBox* text = &textBoxes[0];

        char* cursorItr = text->cursor;

        // Start rendering until the text box is over
        while (x < expectedWidth) {
            //*buf++ = ((uintptr_t)(dbase + *(cursorItr++) * FONT_HEIGHT * FONT_WIDTH_WORDS));
            *buf++ = ((uintptr_t)(dbase + *cursorItr * FONT_HEIGHT * FONT_WIDTH_WORDS));

            x += 1;
        }


        // Put out 4 transparent pixels if no text is visible here
        //todo: color run of transparent pixels between text boxes
        //*buf++ = (uintptr_t)tokTransparents;
    }

    *buf++ = ((uintptr_t)(tokTextLineEnd));
    *buf++ = 0;

    dest->data_used = buf - dest->data;

    // Set size of each token array
    dest->fragment_words = FRAGMENT_WORDS;

    dest->status = SCANLINE_OK;
}

void setWholeScreenDocument() {
    TextBox& box = textBoxes[textBoxesCount++];
    box.initWholeScreen();

    memset(box.text, '\0', sizeof(box.text));
    memset(box.text, 'A', sizeof(box.text));
    box.text[sizeof(box.text) - 1] = '\0';
    //memcpy(box.text, "Type something here...", 22);

    box.x_max /= 8;
}