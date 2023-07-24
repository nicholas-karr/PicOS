#include <iostream>
#include <iomanip>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <format>

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdint>

unsigned char* readBmp(const char* filename)
{
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];

    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, f);

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    // allocate 3 bytes per pixel
    int size = 3 * width * height;
    unsigned char* data = new unsigned char[size];

    // read the rest of the data at once
    fread(data, sizeof(unsigned char), size, f);
    fclose(f);

    for (i = 0; i < size; i += 3)
    {
        // flip the order of every 3 bytes
        unsigned char tmp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = tmp;
    }

    return data;
}

#define OPAQUE_MASK (1 << 5)
using uchar = unsigned char;

int main() {
    auto buf = readBmp("../assets/pointer.bmp");

    uint16_t out[15 * 9];

    for (int i = 0; i < 15 * 9; i++) {
        int r = buf[i * 3 + 0];
        int g = buf[i * 3 + 1];
        int b = buf[i * 3 + 2];

        if (((r + g + b) / 3) > (255 / 2)) {
            out[i] = 0xFFFF;
        }
        else {
            out[i] = 0;
        }

        std::cout << "0x" << std::hex << out[i] << ", ";
    }

    

    return 0;
}
