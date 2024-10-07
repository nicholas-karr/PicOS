#ifndef PICOS_DRAW_H
#define PICOS_DRAW_H

// Number of pixels to insert to the left of the screen
// Can be changed at runtime through setHorizontalCalibration
#define HCAL_DEFAULT 50

#define COLOR_HCAL 0
#define COLOR_TRANSPARENT (1u << 5u)

#define SCREEN_WIDTH 1280

// Represents one of the 1-3 layers rendered onscreen
// Transparent pixels will fall back to the value of the layer behind them
struct Layer {
    // Reference to an array of 32 bit words that are either DMA target pointers or render instructions
    uint32_t*& data;

    // Number of 16 bit words filled in data
    uint16_t& data_used;
};

extern __not_in_flash("z") uint16_t tokBackground[];

// Fragment that starts a line
// Inserts 3 transparent pixels
extern __not_in_flash("x") uint16_t tokTextLineBegin[];

// Fragment that ends a line
extern __not_in_flash("y") uint16_t tokLineEnd[];

// A fragment of just transparent pixels
extern __not_in_flash("y") uint16_t tokTransparents[];

// Converts between ASCII and offsets in the font sprites
extern __not_in_flash("z") char fontConv[255];

extern void makeFontConvTable();

// Fill the back layer with the background at scanline y
void __time_critical_func(drawBackground) (uint16_t y, Layer background);

void setHorizontalCalibration(uint16_t hcal);

uint16_t getHorizontalCalibration();

#endif // ifndef PICOS_DRAW_H