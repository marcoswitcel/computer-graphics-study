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

struct Texture2D
{
    unsigned int width;
    unsigned int height;
    std::vector<S_RGB> buffer;
};

class ZBuffer
{
public:
    unsigned int width;
    unsigned int height;
    std::vector<float> buffer;
    ZBuffer(unsigned width, unsigned height): width(width), height(height), buffer(width * height)
    {
        clearForUse();
    }

    void clearForUse()
    {
        for (int i = width * height; i--;)
        {
            buffer[i] = -std::numeric_limits<float>::max();
        }
    }
};

#endif // COLOR_AND_IMAGE_H
