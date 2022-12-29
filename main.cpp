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

inline void drawLine(FrameBuffer &frameBuffer, Vec2i a, Vec2i b, S_RGB color)
{
    drawLine2(frameBuffer, a.x, a.y, b.x, b.y, color);
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

void triangle(FrameBuffer &frameBuffer, Vec2i a, Vec2i b, Vec2i c, S_RGB color)
{
    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);

    assert(a.y <= c.y && "deveria ser menor ou igual sempre");
    /* drawLine(frameBuffer, a, b, S_RGB { 255, 0, 0 });
    drawLine(frameBuffer, b, c, S_RGB { 0, 255, 0 });
    drawLine(frameBuffer, c, a, S_RGB { 0, 0, 255 }); */
    drawLine(frameBuffer, a, b, color);
    drawLine(frameBuffer, b, c, color);
    drawLine(frameBuffer, c, a, color);
}

void triangle2(FrameBuffer &frameBuffer, Vec2i a, Vec2i b, Vec2i c, S_RGB color)
{
    auto &buffer = frameBuffer.buffer;

    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);

    assert(a.y <= c.y && "deveria ser menor ou igual sempre");

    int totalHeight = c.y -  a.y;
    for (int y = a.y; y < b.y; y++)
    {
        int segmentHeight = b.y - a.y + 1;
        float alpha = (float) (y - a.y) / totalHeight;
        float beta  = (float) (y - a.y) / segmentHeight;

        Vec2i p0 = {
            .x = a.x + ((int) ((c.x - a.x) * alpha)),
            .y = a.y + ((int) ((c.y - a.y) * alpha)),
        };

        Vec2i p1 = {
            .x = a.x + ((int) ((b.x - a.x) * beta)),
            .y = a.y + ((int) ((b.y - a.y) * beta)),
        };

        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t color_index = y * frameBuffer.width + j;

            if (color_index > buffer.size()) {
                continue;
            };

            buffer[color_index] = color;
        }
    }

    for (int y = b.y; y <= c.y; y++)
    {
        int segmentHeight = c.y - b.y + 1;
        float alpha = (float) (y - a.y) / totalHeight;
        float beta  = (float) (y - b.y) / segmentHeight;
        
        Vec2i p0 = {
            .x = a.x + ((int) ((c.x - a.x) * alpha)),
            .y = a.y + ((int) ((c.y - a.y) * alpha)),
        };

        Vec2i p1 = {
            .x = b.x + ((int) ((c.x - b.x) * beta)),
            .y = b.y + ((int) ((c.y - b.y) * beta)),
        };


        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t color_index = y * frameBuffer.width + j;

            if (color_index > buffer.size()) {
                continue;
            };

            buffer[color_index] = color;
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

void renderTeapotScene()
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

    ObjModel* model = ObjModel::readObjModel("teapot.obj");

    drawRect(frameBuffer, 10, 10, 700, 100, S_RGB { 255, 255, 0 });
    drawRect(frameBuffer, 20, 20, 150, 730, S_RGB { 0, 255, 255 });

    drawWireframe(model, &frameBuffer);

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

void renderTriangleTestScene()
{
    constexpr unsigned width = 600;
    constexpr unsigned height = 400;

    const auto RED = S_RGB { 255, 0, 0 };
    const auto GREEN = S_RGB { 0, 255, 0 };
    const auto WHITE = S_RGB { 255, 255, 255 };
    const auto BLACK = S_RGB { 0, 0, 0 };


    FrameBuffer frameBuffer = {
        width: width,
        height: height,
        buffer: vector<S_RGB>(width * height),
    };

    Vec2i a = { .x = 100, .y = 100 };
    Vec2i b = { .x = 150, .y = 150 };
    Vec2i c = { .x = 200, .y = 100 };

/*     triangle(frameBuffer, a, b, c, S_RGB { 255, 0, 0 }); */

    fill(frameBuffer, BLACK);
    Vec2i t0[3] = {Vec2i { .x = 10, .y=70 },   Vec2i { .x = 50, .y=160 },  Vec2i { .x = 70, .y=80 }}; 
    Vec2i t1[3] = {Vec2i { .x = 180, .y=50 },  Vec2i { .x = 150, .y=1 },   Vec2i { .x = 70, .y=180 }}; 
    Vec2i t2[3] = {Vec2i { .x = 180, .y=150 }, Vec2i { .x = 120, .y=160 }, Vec2i { .x = 130, .y=180 }}; 
    // triangle(frameBuffer, t0[0], t0[1], t0[2], RED); 
    // triangle(frameBuffer, t1[0], t1[1], t1[2], WHITE); 
    // triangle(frameBuffer, t2[0], t2[1], t2[2], GREEN);

    triangle2(frameBuffer, t0[0], t0[1], t0[2], RED); 
    triangle2(frameBuffer, t1[0], t1[1], t1[2], WHITE); 
    triangle2(frameBuffer, t2[0], t2[1], t2[2], GREEN);

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

int main(int argc, char *argv[])
{
    //renderTeapotScene();
    renderTriangleTestScene();

    return 0;
}

