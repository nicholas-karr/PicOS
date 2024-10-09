#ifndef PICOS_FONT_H
#define PICOS_FONT_H

#include <stdlib.h>

#include "pico/types.h"

// Standard LVGL font declaration
typedef struct {
    uint16_t bitmap_index;
    uint16_t adv_w;
    int8_t box_w, box_h, ofs_x, ofs_y;
} __attribute__((packed)) lv_font_fmt_txt_glyph_dsc_t;

typedef struct {
    uint16_t range_start, range_length, glyph_id_start, list_length;
    void *unicode_list, *glyph_id_ofs_list;
    enum {
        LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    } type;
} lv_font_fmt_txt_cmap_t;

typedef struct {
    const uint8_t *glyph_bitmap;
    const lv_font_fmt_txt_glyph_dsc_t *glyph_dsc;
    const lv_font_fmt_txt_cmap_t *cmaps;
    uint8_t cmap_num, bpp, kern_scale, kern_classes;
    void *kern_dsc;
} lv_font_fmt_txt_dsc_t;

typedef struct {
    lv_font_fmt_txt_dsc_t *dsc;
    uint8_t line_height, base_line;
} lv_font_t;

extern const lv_font_t ubuntu_mono8;
extern const lv_font_t lcd;

extern const lv_font_t* font;
extern uint32_t *font_raw_pixels;

// 4x32 bit words per fragment passed to the renderer
#define FRAGMENT_WORDS 4

#define FONT_WIDTH FRAGMENT_WORDS
#define FONT_HEIGHT 15
#define FONT_SIZE_WORDS (FONT_HEIGHT * FRAGMENT_WORDS)

// Transform from an LVGL font to a constant-width bitmap
void build_font();

#endif // ifndef PICOS_FONT_H