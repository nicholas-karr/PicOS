#ifndef PICOS_TEXT_H
#define PICOS_TEXT_H

#include "draw.h"

void __time_critical_func(renderTextBoxes) (uint16_t y, Layer text);

#define CHAR_TRANSPARENT '\x7F'
#define FONT_CHAR_TRANSPARENT ('\x7F' - 0x20)

// Converts between ASCII and offsets in the font sprites
extern __not_in_flash("z") std::array<uint8_t, 255> fontConv;

#endif // ifndef PICOS_TEXT_H