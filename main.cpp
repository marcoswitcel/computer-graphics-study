#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>

#include "color_and_image.h"

using namespace std;

void saveFrameBuffertoPPMFile(const FrameBuffer &frameBuffer, const char *filename)
{
    auto dimx = frameBuffer.width, dimy = frameBuffer.height;
    ofstream ofs(filename, ios_base::out | ios_base::binary);

    ofs << "P6" << endl
        << dimx << ' ' << dimy << endl
        << "255" << endl;

    for (const S_RGB &color : frameBuffer.buffer)
    {
        ofs << (char)(color.r) << (char)(color.g) << (char)(color.b);
    }

    ofs.close();
}

/**
 * @todo João, verificar as bordas para renderizar direito e corrigir problemas de overlfow
 * @brief Desenha um retângulo em ângulo reto
 * 
 * @param frameBuffer 
 * @param xStart 
 * @param yStart 
 * @param rectWidth 
 * @param rectHeight 
 * @param color 
 */
void drawRect(FrameBuffer &frameBuffer, uint32_t xStart, uint32_t yStart, uint32_t rectWidth, uint32_t rectHeight, S_RGB color)
{
    auto &buffer = frameBuffer.buffer;
    for (auto currentX = xStart; currentX < xStart + rectWidth; currentX++)
    {
        for (auto currentY = yStart; currentY < yStart + rectHeight; currentY++)
        {
            auto &rgb = buffer[currentY * frameBuffer.width + currentX];
            rgb = color;
        }   
    }
}

void drawLine(FrameBuffer &frameBuffer, uint32_t xStart, uint32_t yStart, uint32_t xEnd, uint32_t yEnd, S_RGB color)
{
    auto &buffer = frameBuffer.buffer;

    for (float t = 0.0; t < 1.0; t += 0.1)
    {
        auto x = xStart + (xEnd - xStart) * t;
        auto y = yStart + (yEnd - yStart) * t;

        auto &rgb = buffer[y * frameBuffer.width + x];
        rgb = color;
    }
}

int main(int argc, char *argv[])
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 768;
    const int fov = M_PI / 2.;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    for (S_RGB &rgb : frameBuffer.buffer)
    {
        rgb.r = 255;
        rgb.g = 0;
        rgb.b = 0;
    }

    drawRect(frameBuffer, 10, 10, 250, 75, S_RGB { 255, 255, 0 });

    drawRect(frameBuffer, 20, 20, 200, 60, S_RGB { 0, 255, 255 });

    drawLine(frameBuffer, 10, 20, 300, 300, S_RGB { 255, 255, 255 });

    saveFrameBuffertoPPMFile(frameBuffer, "image.ppm");

    return 0;
}

