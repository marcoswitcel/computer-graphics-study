#ifndef TEXT_RENDERING_CPP
#define TEXT_RENDERING_CPP

#include "./color_and_image.h"

static uint8_t font[] = {
    0b01111110,
    0b01111110,
    0b01100110,
    0b01111110,
    0b01111110,
    0b01100110,
    0b01100110,
    0b00000000,
}; 

Texture2D createGlyph()
{
    Texture2D glyph = {
        width : 8,
        height : 8,
        buffer : std::vector<S_RGB>(8 * 8),
    };

    // @todo João, falta a implementação correta aqui
    for (S_RGB &rgb : glyph.buffer)
    {
        rgb = { 255, 0, 0, };
    }

    return glyph; 
}

#endif // TEXT_RENDERING_CPP
