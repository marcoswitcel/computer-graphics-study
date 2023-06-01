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
    S_RGB fillColor = { 255, 0, 0, };
    S_RGB bgColor = { 0, 255, 0, };

    Texture2D glyph = {
        width : 8,
        height : 8,
        buffer : std::vector<S_RGB>(8 * 8),
    };

    for (uint8_t i = 8; i--;)
    {
        uint8_t data = font[i];
        glyph.buffer[i * 8 + 0] = (data & 0b10000000) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 1] = (data & 0b01000000) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 2] = (data & 0b00100000) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 3] = (data & 0b00010000) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 4] = (data & 0b00001000) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 5] = (data & 0b00000100) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 6] = (data & 0b00000010) ? fillColor : bgColor;
        glyph.buffer[i * 8 + 7] = (data & 0b00000001) ? fillColor : bgColor;
    }

    return glyph; 
}

#endif // TEXT_RENDERING_CPP
