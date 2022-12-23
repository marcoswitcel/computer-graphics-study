#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <assert.h>

#include "color_and_image.h"
#include "obj_model.cpp"

using namespace std;

void saveFrameBufferToPPMFile(const FrameBuffer &frameBuffer, const char *filename)
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

void drawLine2(FrameBuffer &frameBuffer, int32_t xStart, int32_t yStart, int32_t xEnd, int32_t yEnd, S_RGB color)
{
    auto &buffer = frameBuffer.buffer;
    bool steep = false; 

    if (std::abs(xStart-xEnd) < std::abs(yStart-yEnd)) {
        std::swap(xStart, yStart);
        std::swap(xEnd, yEnd);
        steep = true;
    }

    if (xStart > xEnd) {
        std::swap(xStart, xEnd);
        std::swap(yStart, yEnd);
    }

    for (uint32_t x = xStart; x <= xEnd; x++)
    {
        float t = (x - xStart) / (float) (xEnd - xStart);
        uint32_t y = yStart * (1. - t) + yEnd * t;

        uint32_t color_index = (steep)
            ? x * frameBuffer.width + y
            : y * frameBuffer.width + x;
        
        if (color_index > buffer.size()) {
            continue;
        };

        auto &rgb = buffer[color_index];
        rgb = color;
    }
}

void drawWireframe(ObjModel* model, FrameBuffer* frameBuffer)
{
    const auto width = frameBuffer->width;
    const auto height = frameBuffer->height;

    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        for (int v = 0; v < 3; v++)
        {
            Vec3f v0 = model->getVert(face[v]);
            Vec3f v1 = model->getVert(face[(v+1)%3]);
            int x0 = (v0.x*150.0) + width/2.0;
            int y0 = (v0.y*150.0) + height/4.0;
            int x1 = (v1.x*150.0) + width/2.0;
            int y1 = (v1.y*150.0) + height/4.0;

            drawLine2(*frameBuffer, x0, y0, x1, y1, S_RGB { 255, 255, 255 });
        }
    }
}

void fill(FrameBuffer &frameBuffer, S_RGB color)
{
    for (S_RGB &rgb : frameBuffer.buffer)
    {
        rgb = color;
    }
}

void flipImageInXAxis(FrameBuffer &frameBuffer)
{
    auto &buffer = frameBuffer.buffer;
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;
    const unsigned int halfHeight = frameBuffer.height / 2;
    const bool even = (frameBuffer.height % 2) == 0;

    assert(width * height <= buffer.size());
    if (width * height > buffer.size()) return;

    S_RGB temp;
    for (int row = 0; row < halfHeight; row++)
    {
        for (int col = 0; col < width; col++)
        {
            auto &upperColor  = buffer[row * width + col];
            auto &bottomColor  = buffer[(height - row - 1) * width + col];
            temp = upperColor;
            upperColor = bottomColor;
            bottomColor = temp;
        }   
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

    const auto RED = S_RGB { 255, 0, 0 };
    const auto WHITE = S_RGB { 255, 255, 255 };
    const auto BLACK = S_RGB { 0, 0, 0 };

    fill(frameBuffer, BLACK);

    /* drawRect(frameBuffer, 10, 10, 250, 75, S_RGB { 255, 255, 0 });
    drawRect(frameBuffer, 20, 20, 200, 60, S_RGB { 0, 255, 255 }); */

    ObjModel* model = ObjModel::readObjModel("teapot.obj");

    drawWireframe(model, &frameBuffer);

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");

    return 0;
}

