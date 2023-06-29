#include "fontbuild.h"
#include "screen.h"
#include "ringbuf.h"
#include "textbox-decls.h"

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

extern "C" {
void render_relevant_text_boxes(uint32_t* buf, uint32_t* font, TextBox** relevant);
}

// Fill layer 0 with text
void __time_critical_func(renderTextBoxes) (scanvideo_scanline_buffer* dest, int y) {
    constexpr int expectedWidth = 160; //160 //(1280 / (8 * 2));

    TextBox* relevant[textBoxesCount];
    TextBox** lastRelevant = relevant + 1;
    int relevantCount = 0;

    // Store which text boxes are on this scanline
    for (int i = 0; i < textBoxesCount; ++i) {
        TextBox& test = textBoxes[i];
        if (y >= test.y & y <= test.y_max) {
            relevant[relevantCount++] = &test;
        }
    }

    // sort by x_min


    // Pointer to an array of pointers to tokens/colors
    uint32_t* buf = dest->data;

    *buf++ = ((uintptr_t)(tokTextLineBegin));

    uint32_t* dbase = font_raw_pixels + FONT_WIDTH_WORDS * (y % FONT_HEIGHT);

    //render_relevant_text_boxes(buf, dbase, relevant);
    //buf += expectedWidth;

    render_relevant_text_boxes(buf, dbase, nullptr);
    buf += 160;

    /*for (uint32_t x = 0; x < expectedWidth; x += 1) {
        for (TextBox** i = relevant; i < lastRelevant; ++i) {
            TextBox* text = *i;
            if (x >= text->x & x <= text->x_max) {
                char* cursorItr = text->cursor;

                // Start rendering until the text box is over
                while (x < expectedWidth) {
                    //*buf++ = ((uintptr_t)(dbase + *(cursorItr++) * FONT_HEIGHT * FONT_WIDTH_WORDS));
                    //*buf++ = ((uintptr_t)(dbase + *cursorItr * FONT_HEIGHT * FONT_WIDTH_WORDS));
                    const int v =  'A' * FONT_HEIGHT * FONT_WIDTH_WORDS;
                    *buf++ = ((uintptr_t)(dbase + v));

                    x += 1;
                    //render_relevant_text_boxes((uint32_t*)&x, nullptr, nullptr);
                }
            }
        }

        // Put out 4 transparent pixels if no text is visible here
        //todo: color run of transparent pixels between text boxes
        //*buf++ = (uintptr_t)tokTransparents;
    }*/

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