#include "font.h"

#include "pico.h"
#include "pico/scanvideo.h"

const lv_font_t* font = &ubuntu_mono8;
uint32_t *font_raw_pixels;

// Transform from an LVGL font to a constant-width bitmap
void build_font() {
    uint16_t colors[16];
    for (int i = 0; i < count_of(colors); i++) {
        //colors[i] = PICO_SCANVIDEO_PIXEL_FROM_RGB5(1, 1, 1) * (((16 - i) * 3) / 2);
        
        // dark
        colors[i] = PICO_SCANVIDEO_PIXEL_FROM_RGB5(1, 1, 1) * ((i * 3) / 2);
    }

    font_raw_pixels = (uint32_t *) calloc(4, (font->dsc->cmaps->range_length + 1) * FONT_SIZE_WORDS);

    uint32_t *p = font_raw_pixels;
    assert(font->line_height == FONT_HEIGHT);
    for (int c = 0; c < font->dsc->cmaps->range_length; c++) {
        // inefficient but simple
        const lv_font_fmt_txt_glyph_dsc_t *g = &font->dsc->glyph_dsc[c + 1];
        const uint8_t *b = font->dsc->glyph_bitmap + g->bitmap_index;
        int bi = 0;
        for (int y = 0; y < FONT_HEIGHT; y++) {
            int ey = y - FONT_HEIGHT + font->base_line + g->ofs_y + g->box_h;
            for (int x = 0; x < FRAGMENT_WORDS * 2; x++) {
                uint32_t pixel;
                int ex = x - g->ofs_x;
                if (ex >= 0 && ex < g->box_w && ey >= 0 && ey < g->box_h) {
                    pixel = bi & 1 ? colors[b[bi >> 1] & 0xf] : colors[b[bi >> 1] >> 4];
                    
                    bi++;
                } else {
                    pixel = 0; //PICO_SCANVIDEO_PIXEL_FROM_RGB5(31, 31, 31);
                }

                pixel = ~pixel;

                pixel |= PICO_SCANVIDEO_ALPHA_MASK;

                pixel = pixel & 0xFFFF;

                if (!(x & 1)) {
                    *p = pixel;
                } else {
                    *p++ |= pixel << 16;
                }
            }
            if (ey >= 0 && ey < g->box_h) {
                for (int x = FRAGMENT_WORDS * 2 - g->ofs_x; x < g->box_w; x++) {
                    bi++;
                }
            }
        }
    }

    // Transparent character at 96
    const lv_font_fmt_txt_glyph_dsc_t *g = &font->dsc->glyph_dsc[0];
    int bi = 0;
    for (int y = 0; y < FONT_HEIGHT; y++) {
        int ey = y - FONT_HEIGHT + font->base_line + g->ofs_y + g->box_h;
        for (int x = 0; x < FRAGMENT_WORDS * 2; x++) {
            if (!(x & 1)) {
                *p = 0;
            } else {
                *p++ |= 0;
            }
        }
        if (ey >= 0 && ey < g->box_h) {
            for (int x = FRAGMENT_WORDS * 2 - g->ofs_x; x < g->box_w; x++) {
                bi++;
            }
        }
    }
}