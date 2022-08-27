#ifndef COLOR_AND_IMAGE_H
#define COLOR_AND_IMAGE_H

#include <stdint.h>
#include <vector>

struct S_RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct FrameBuffer
{
    unsigned int width;
    unsigned int height;
    std::vector<S_RGB> buffer;
};

#endif // COLOR_AND_IMAGE_H
