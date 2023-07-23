#ifndef PICOS_TEXTBOX_H
#define PICOS_TEXTBOX_H

#include <vector>
#include <string>
#include <bit>
#include <string_view>
#include <algorithm>

#include "fontbuild.h"
#include "screen.h"
#include "ringbuf.h"
#include "textbox-decls.h"

#define STR_ALIGNED(S) ( (_Alignas(4) char []){ S } )

#define ROUND_UP(num, multiple) (((num + multiple - 1) / multiple) * multiple)
#define ROUND_DOWN(num, multiple) (num >= 0 ? (num / multiple) * multiple : ((num - multiple + 1) / multiple) * multiple)


//  __attribute__((__packed__)) alignas(4)
struct TextLine {
    size_t stringSize_ = 0;
    char* data_ = internalBuf_;

    size_t allocLen_ = sizeof(internalBuf_);
    
    char internalBuf_[16] = { };

    TextLine() { 
        
    }
    TextLine(size_t allocationHint) {
        assert(false);

        if (allocationHint <= sizeof(internalBuf_)) {

        }
        else {
            allocLen_ = ROUND_UP(allocationHint, 4);
            data_ = (char*)aligned_alloc(4, allocLen_);
            memset(data_, 0, allocLen_);
        }
    }

    size_t size() {
        return stringSize_;
    }

    void reserve(size_t cnt) {
        if (allocLen_ >= cnt) { return; }

        cnt = std::bit_ceil(cnt);
        allocLen_ = ROUND_UP(cnt, 4);

        if (data_ == internalBuf_) {
            //allocLen_ = ROUND_UP(cnt, 4);
            data_ = (char*)aligned_alloc(4, allocLen_);
            memset(data_, 0, allocLen_);
            memcpy(data_, internalBuf_, stringSize_);
        }
        else {
            //allocLen_ = ROUND_UP(cnt, 4);
            char* tmp = (char*)aligned_alloc(4, allocLen_);
            memset(tmp, 0, allocLen_);
            memcpy(tmp, data_, stringSize_);
            free(data_);
            data_ = tmp;
        }

        //padEnd();
    }

    void padEnd() {
        switch (allocLen_ % 4) {
            case 3: data_[allocLen_ - 1] = 0; //'\x7F' - 0x20;
            case 2: data_[allocLen_ - 2] = 0; //'\x7F' - 0x20;
            case 1: data_[allocLen_ - 3] = 0; //'\x7F' - 0x20;
            // case 0: Aligned already
        }
    }

    void push_back(char c) {
        reserve(stringSize_ + 1);
        stringSize_++;
        data_[stringSize_ - 1] = c;

        
    }

    char* data() {
        return data_;
    }

    // How many characters this line will produce, including transparent trailers
    size_t alignedSize() {
        return ROUND_UP(stringSize_, 4);
    }

    ~TextLine() {
        if (data_ != internalBuf_) {
            free(data_);
        }
    }
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

struct __attribute__((__packed__)) alignas(4) TextLineEntry {
    const char* buf;
    const uint32_t* c_end;
    uint32_t* font;
};

struct TextBox {
    uint16_t x, x_max;
    uint16_t y, y_max;

    std::pair<size_t, size_t> cursor; // Where user is writing [line, pos]

    bool editable = true;

    std::vector<TextLine> lines;
    int topOfWindow = 0; // Which line is the first visible

    void initWholeScreen() {
        //x = y = 0;
        //x_max = screen.x_end;
        //y_max = screen.y_end;
    }

    void fromString(std::string_view str) {
        size_t linecnt = std::count(str.begin(), str.end(), '\n') + 1;
        lines.reserve(linecnt);
        lines.emplace_back();

        for (auto i : str) {
            if (i == '\n') {
                lines.emplace_back();
            }
            else if (i < 0x20 || i == 0) {
                assert(false);
            }
            else {
                lines.back().push_back(i - 0x20); //
            }
        }
    }

    size_t size() {
        size_t ret;
        for (auto& i : lines) {
            ret += i.size() + 1; // include newline
        }
        return ret;
    }

    std::string toString() {
        size_t sz = size();
        std::string str(sz, '\0');
        str.resize(0);
        for (auto& i : lines) {
            str.append(i.data());
            str.push_back('\n');
        }
        assert(str.size() == sz);
        return str;
    }

    void init(uint16_t x_, uint16_t x_max_, uint16_t y_, uint16_t y_max_, const char* text_, bool editable_) {
        x = ROUND_UP(x_, 4);
        x_max = ROUND_UP(x_max_, 4);
        y = ROUND_UP(y_, 4);
        y_max = ROUND_UP(y_max_, 4);
        editable = editable_;

        if (editable_) {
            fromString(text_);
        }
        else {
            //todo: ensure packing, do string view
            fromString(text_);
        }
    }

    int getWidth() { return x_max - x; }
    int getHeight() { return y_max - y; }

    bool empty() {
        return lines.empty();
    }

    TextLine* lineAt(int y) {
        y -= this->y;
        //auto index = topOfWindow + ROUND_DOWN(y, FONT_HEIGHT);
        //int index = topOfWindow + (y - (y % FONT_HEIGHT)) / FONT_HEIGHT;
        int index = y / FONT_HEIGHT;
        if (index < 0 || index >= lines.size() || lines.at(index).stringSize_ == 0) {
            return nullptr;
        }
        return &lines[index];
    }
};

static TextBox textBoxes[10];
static int textBoxesCount = 0;

static RingBuffer<TextLinePreamble, 32> preambles;

// PUT THIS INSIDE THE SCANLINE BUF
uint32_t* getPreamble(int length) {
    TextLinePreamble* i = preambles.alloc();
    i->setFollowingSize(length);
    return (uint32_t*)(i->buf);

    //return (uint32_t*)(preambles.buf[0].buf);
}

extern "C" {
void render_relevant_text_boxes(uint32_t* buf, uint32_t* font, TextLineEntry* relevant);
}
extern "C" char* testfunc(uint32_t* buf, uint32_t* font, TextBox** relevant);
volatile char* ret;

//static const char stocktext[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. \nMorbi sed lacus diam. Ut vitae nisl massa. Donec pretium nulla metus, vel pretium dolor volutpat ac. In a libero eget lorem bibendum porta nec eu ex. Suspendisse lo\nbortis, sem eget facilisis varius, velit magna dapibus risus, eget aliquet ligula mi at massa. Fusce ut lacinia turpis. Donec tortor augue, ultricies ac ante a, molestie porttitor purus. In vel interd\num orci. In eget eros scelerisque, consequat leo vel, aliquet nulla. Aenean quis finibus lorem.";
static const char stocktext[] = "LOREM IPSUM DOLOR SIT AMET CONSECTETUR ADIPISCING ELIT MORBI SED LACUS DIAM UT VITAE NISL MASSA. DONEC PRETIUM NULLA METUS VEL PRETIUM DOLOR VOLUTPAT AC. IN A LIBERO EGET LOREM BIBENDUM PORTA NEC EU EX. SUSPENDISSE LOBORTIS, SEM EGET FACILISIS VARIUS, VELIT MAGNA DAPIBUS RISUS, EGET ALIQUET LIGULA MI AT MASSA. FUSCE UT LACINIA TURPIS. DONEC TORTOR AUGUE, ULTRICIES AC ANTE A, MOLESTIE PORTTITOR PURUS. IN VEL INTERDUM ORCI. IN EGET EROS SCELERISQUE, CONSEQUAT LEO VEL, ALIQUET NULLA. AENEAN QUIS FINIBUS LOREM.";

#define CHARACTERS_IN_SCREEN (1280 / 4)

// Ends with 0-3 transparent characters for the end of the window
// 4 end-to-start arrays
static char padEnd[4 * CHARACTERS_IN_SCREEN];

void loadPadEnd() {
    memset(padEnd, 0, CHARACTERS_IN_SCREEN);
    memset(padEnd + CHARACTERS_IN_SCREEN, 0, CHARACTERS_IN_SCREEN);
    memset(padEnd + CHARACTERS_IN_SCREEN * 2, 0, CHARACTERS_IN_SCREEN);
    memset(padEnd + CHARACTERS_IN_SCREEN * 3, 0, CHARACTERS_IN_SCREEN);

    memset(padEnd + CHARACTERS_IN_SCREEN * 2 - 4, '\x7F' - 0x20, 3);
    memset(padEnd + CHARACTERS_IN_SCREEN * 3 - 3, '\x7F' - 0x20, 2);
    memset(padEnd + CHARACTERS_IN_SCREEN * 4 - 2, '\x7F' - 0x20, 1);
}

struct Layer {
    uint32_t*& data;
    uint16_t& data_used;
};


// Fill layer 0 with text
void __time_critical_func(renderTextBoxes) (uint16_t y, Layer text) {
    //constexpr uint16_t expectedWidth = 160; //160 //(1280 / (8 * 2));
    
    TextBox* relevant[textBoxesCount];
    uint16_t relevantCount = 0;

    // Store which text boxes are on this scanline (probably has to happen every frame?)
    for (uint32_t i = 0; i < textBoxesCount; ++i) {
        TextBox& test = textBoxes[i];
        if (y >= test.y && y <= test.y_max) {
            relevant[relevantCount++] = &test;
        }
    }

    if (relevantCount == 0) {
        uint32_t* buf = text.data;
        
        *buf++ = ((uintptr_t)(tokTextLineBegin));
        text.data_used = (buf - text.data) / 2;
        //return;
    }

    //relevant[relevantCount] = &(textBoxes[0]);

    // sort by x_min
    std::sort(relevant, relevant + relevantCount, [](TextBox* lhs, TextBox* rhs) { return lhs->x < rhs->x; });

    // Pointer to an array of pointers to tokens/colors
    uint32_t* buf = text.data;

    *buf++ = ((uintptr_t)(tokTextLineBegin));

    

    TextLineEntry entries[8] = {};
    uint16_t entriesCnt = 0;
    uint16_t xitr = 0;
    uint32_t* bufItr = buf;

    for (uint16_t i = 0; i < relevantCount && entriesCnt < 8; ++i) {
        const TextBox& box = *relevant[i];

        if (box.x >= screen.x_end) { 
            continue; 
        } // should be setting 'visible' flag too
        
        if (xitr < box.x) {
            // Pre-padding, one character at a time
            size_t length = (box.x - xitr) / 4;
            entries[entriesCnt++] = { (char*)nullptr, bufItr += length, nullptr }; // 4 pixels per 32-bit ptr
            xitr += length * 4;

            if (xitr != box.x) {
                assert(false);
                __breakpoint();
            }
        }

        auto line = relevant[i]->lineAt(y);      //(y);
        if (line) {
            size_t length = line->alignedSize();
            uint16_t localY = y - relevant[i]->y;
            char* data = line->data();
            entries[entriesCnt++] = { line->data(), bufItr += length, font_raw_pixels + FONT_WIDTH_WORDS * (localY % FONT_HEIGHT) }; // todo: replace aligned size with unaligned and use geq in asm
            xitr += length * 4;
        }

        uint16_t lastX = std::min(box.x_max, screen.x_end);
        if (xitr + 4 < lastX ) {
            size_t length = lastX - xitr;
            length /= 4;
            length = ROUND_UP(length, 4);
            entries[entriesCnt++] = { padEnd, bufItr += length, font_raw_pixels };
            xitr += length * 4;
        }
    }

    /*if (bufItr <= buf + 160)*/ 
    entries[entriesCnt++] = { (char*)nullptr, buf + 160, nullptr }; // Fill in the rest of the line with transparent pixels
    entries[entriesCnt++] = { (char*)nullptr, nullptr, nullptr }; // End of line meta token

    uint32_t* font = font_raw_pixels; // + FONT_WIDTH_WORDS * (y % FONT_HEIGHT);

    render_relevant_text_boxes(buf, font, entries);
    buf += 160;

    *buf++ = ((uintptr_t)(tokTextLineEnd));
    *buf++ = 0;

    text.data_used = (buf - text.data) / 2;

    // Set size of each token array
    //dest->fragment_words = FRAGMENT_WORDS;
}

#endif