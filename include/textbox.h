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

#define ROUND_UP(num, multiple) ((num + multiple - 1) & -multiple)
#define ROUND_DOWN(num, multiple) (num >= 0 ? (num / multiple) * multiple : ((num - multiple + 1) / multiple) * multiple)


struct __attribute__((__packed__)) alignas(4) TextLine {
    size_t stringSize_ = 0;
    char* data_ = internalBuf_;

    size_t allocLen_ = sizeof(internalBuf_);
    
    char internalBuf_[16];

    TextLine() { stringSize_ = 0; }
    TextLine(size_t allocationHint) {
        stringSize_ = 0;
        allocLen_ = ROUND_UP(allocationHint, 4);
        data_ = (char*)calloc(allocLen_, 1);
    }

    size_t size() {
        return stringSize_;
    }

    void reserve(size_t cnt) {
        if (allocLen_ >= cnt) { return; }

        cnt = std::bit_ceil(cnt);

        if (data_ == internalBuf_) {
            data_ = (char*)calloc(ROUND_UP(cnt, 4), 1);
            memcpy(data_, internalBuf_, stringSize_);
            allocLen_ = ROUND_UP(cnt, 4);
        }
        else {
            char* tmp = (char*)calloc(ROUND_UP(cnt, 4), 1);
            memcpy(tmp, data_, stringSize_);
            free(data_);
            data_ = tmp;
            allocLen_ = ROUND_UP(cnt, 4);
        }
    }

    void push_back(char c) {
        reserve(stringSize_ + 1);
        data_[stringSize_++] = c;
    }

    char* data() {
        return data_;
    }

    size_t alignedSize() {
        size_t p1 = this->stringSize_ + 4;
        size_t p3 = this->stringSize_ + 1;
        size_t p2 = this->stringSize_ % 4;
        //printf("%d\r\n", p1 + p2);
        return ROUND_UP(stringSize_, 4);
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

struct __attribute__((__packed__)) TextLineEntry {
    const char* buf;
    const uint32_t* c_end;
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
            else {
                lines.back().push_back(i);
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

    void init(uint16_t x_, uint16_t x_max_, uint16_t y_, uint16_t y_max_, char* text_, bool editable_) {
        x = x_;
        x_max = x_max_;
        y = y_;
        y_max = y_max_;
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
        auto index = topOfWindow;// + ROUND_DOWN(y, FONT_HEIGHT);
        if (index < 0 || index >= lines.size()) {
            return nullptr;
        }
        return &lines[index];
    }
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

extern "C" {
void render_relevant_text_boxes(uint32_t* buf, uint32_t* font, TextLineEntry* relevant);
}
extern "C" char* testfunc(uint32_t* buf, uint32_t* font, TextBox** relevant);
volatile char* ret;

//static const char stocktext[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. \nMorbi sed lacus diam. Ut vitae nisl massa. Donec pretium nulla metus, vel pretium dolor volutpat ac. In a libero eget lorem bibendum porta nec eu ex. Suspendisse lo\nbortis, sem eget facilisis varius, velit magna dapibus risus, eget aliquet ligula mi at massa. Fusce ut lacinia turpis. Donec tortor augue, ultricies ac ante a, molestie porttitor purus. In vel interd\num orci. In eget eros scelerisque, consequat leo vel, aliquet nulla. Aenean quis finibus lorem.";
static const char stocktext[] = "LOREM IPSUM DOLOR SIT AMET CONSECTETUR ADIPISCING ELIT MORBI SED LACUS DIAM UT VITAE NISL MASSA. DONEC PRETIUM NULLA METUS VEL PRETIUM DOLOR VOLUTPAT AC. IN A LIBERO EGET LOREM BIBENDUM PORTA NEC EU EX. SUSPENDISSE LOBORTIS, SEM EGET FACILISIS VARIUS, VELIT MAGNA DAPIBUS RISUS, EGET ALIQUET LIGULA MI AT MASSA. FUSCE UT LACINIA TURPIS. DONEC TORTOR AUGUE, ULTRICIES AC ANTE A, MOLESTIE PORTTITOR PURUS. IN VEL INTERDUM ORCI. IN EGET EROS SCELERISQUE, CONSEQUAT LEO VEL, ALIQUET NULLA. AENEAN QUIS FINIBUS LOREM.";

// Fill layer 0 with text
void __time_critical_func(renderTextBoxes) (scanvideo_scanline_buffer* dest, uint16_t y) {
    constexpr uint16_t expectedWidth = 160; //160 //(1280 / (8 * 2));
    
    TextBox* relevant[textBoxesCount];
    TextBox** lastRelevant = relevant + 1;
    uint16_t relevantCount = 0;

    // Store which text boxes are on this scanline (probably has to happen every frame?)
    for (uint16_t i = 0; i < textBoxesCount; ++i) {
        TextBox& test = textBoxes[i];
        if (y >= test.y && y <= test.y_max) {
            relevant[relevantCount++] = &test;
        }
    }

    //relevant[relevantCount] = &(textBoxes[0]);

    // sort by x_min
    std::sort(relevant, relevant + relevantCount, [](TextBox* lhs, TextBox* rhs) { return lhs->x < rhs->x_max; });

    // Pointer to an array of pointers to tokens/colors
    uint32_t* buf = dest->data;

    *buf++ = ((uintptr_t)(tokTextLineBegin));

    

    TextLineEntry entries[8] = {};
    uint16_t entriesCnt = 0;
    uint16_t xitr = 0;
    uint32_t* bufItr = buf;

    for (uint16_t i = 0; i < relevantCount && entriesCnt < 8; ++i) {
        if (xitr < relevant[i]->x) {
            // Pre-padding
            entries[entriesCnt++] = { (char*)nullptr, bufItr += ((relevant[i]->x - xitr) / 4) }; // 4 pixels per 32-bit ptr
            xitr = relevant[i]->x;
        }

        auto line = relevant[i]->lineAt(0);      //(y);
        if (line) {
            size_t length = line->alignedSize();
            entries[entriesCnt++] = { line->data(), bufItr += length }; // todo: replace aligned size with unaligned and use geq in asm
            xitr += length * 4; //length * FONT_WIDTH_WORDS;
        }
    }

    entries[entriesCnt++] = { (char*)nullptr, buf + 160 }; // Fill in the rest of the line with transparent pixels
    entries[entriesCnt++] = { (char*)nullptr, nullptr }; // End of line meta token

    uint32_t* font = font_raw_pixels + FONT_WIDTH_WORDS * (y % FONT_HEIGHT);

    render_relevant_text_boxes(buf, font, entries);
    buf += 160;

    *buf++ = ((uintptr_t)(tokTextLineEnd));
    *buf++ = 0;

    dest->data_used = buf - dest->data;

    // Set size of each token array
    dest->fragment_words = FRAGMENT_WORDS;

    dest->status = SCANLINE_OK;
}