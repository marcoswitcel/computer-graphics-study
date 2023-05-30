#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <assert.h>
#include <chrono>

#include "color_and_image.h"
#include "obj_model.cpp"
#include "image_data.cpp"
#include "text_rendering.cpp"

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

void drawWireframe2(ObjModel* model, FrameBuffer &frameBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        for (int v = 0; v < 3; v++)
        {
            Vec3f v0 = model->getVert(face[v]);
            Vec3f v1 = model->getVert(face[(v+1)%3]);
            int x0 = (v0.x*300.0) + width/2.0;
            int y0 = (v0.y*300.0) + height/2.0;
            int x1 = (v1.x*300.0) + width/2.0;
            int y1 = (v1.y*300.0) + height/2.0;

            drawLine2(frameBuffer, x0, y0, x1, y1, S_RGB { 255, 255, 255 });
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
    for (int y = a.y; y <= b.y; y++)
    {
        int segmentHeight = b.y - a.y + 1;
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
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
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
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

inline Vec3f barycentric(const Vec2i &a, const Vec2i &b, const Vec2i &c, const Vec2i &p)
{
    Vec3f m0 = { (float) (c.x - a.x), (float) (b.x - a.x), (float) (a.x - p.x) };
    Vec3f m1 = { (float) (c.y - a.y), (float) (b.y - a.y), (float) (a.y - p.y) };
    Vec3f u = m0 ^ m1;

    if (std::abs(u.z) < 1) return { -1, 1, 1 };
    return { 1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z };
}

void triangle3(FrameBuffer &frameBuffer, Vec2i a, Vec2i b, Vec2i c, S_RGB color)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;
    auto &buffer = frameBuffer.buffer;


    Vec2i bboxmin = { (int) width - 1, (int) height - 1 };
    Vec2i bboxmax = { 0, 0 };
    Vec2i clamp = { (int) width - 1, (int) height - 1 };


    Vec2i pts[3] = { a, b, c};
    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    Vec2i p;
    for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++)
    {
        for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++)
        {
            Vec3f bsScreen = barycentric(a, b, c, p);
            
            uint32_t colorIndex = p.y * frameBuffer.width + p.x;

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            buffer[colorIndex] = color;
        }
    }
}

Vec3f cross(const Vec3f &a, const Vec3f &b) {
    return Vec3f {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z, 
        .z = a.x * b.y - a.y * b.x,
    };
}

inline Vec3f barycentric(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &p)
{
    Vec3f newA = {0}, newB = {0}, newC = {0};

    newA.x = c.x - a.x;
    newA.y = b.x - a.x;
    newA.z = a.x - p.x;

    newB.x = c.y - a.y;
    newB.y = b.y - a.y;
    newB.z = a.y - p.y;

    newC.x = c.z - a.z;
    newC.y = b.z - a.z;
    newC.z = a.z - p.z;

    Vec3f u = cross(a, b);

    if (std::abs(u.z) > 1e-2) {
        return Vec3f {
            .x = 1.f-(u.x + u.y)/u.z,
            .y = u.y / u.z,
            .z = u.x / u.z,
        };
    }

    return Vec3f { -1, 1, 1 };
}

void triangle3(FrameBuffer &frameBuffer, ZBuffer &zBuffer, Vec3f a, Vec3f b, Vec3f c, S_RGB color)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;
    auto &buffer = frameBuffer.buffer;


    Vec3f bboxmin = {(float) ( width - 1), (float) (height - 1) };
    Vec3f bboxmax = { 0, 0 };
    Vec3f clamp = {(float) ( width - 1), (float) (height - 1) };


    Vec3f pts[3] = { a, b, c};
    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    Vec3f p;
    for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++)
    {
        for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++)
        {
            Vec3f bsScreen = barycentric(a, b, c, p);
            
            uint32_t colorIndex = p.y * frameBuffer.width + p.x;

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            p.z = 0;

            // calcula posição no exio z do pixel atual
            p.z += a.z * bsScreen.x;
            p.z += b.z * bsScreen.y;
            p.z += c.z * bsScreen.z;

            if (zBuffer.buffer[colorIndex] < p.z) {
                zBuffer.buffer[colorIndex] = p.z;
                buffer[colorIndex] = color;
            }
        }
    }
}

static inline Vec2f lerp(Vec2f v0, Vec2f v1, float percent)
{
    return Vec2f {
        .x = v0.x + ((v1.x - v0.x) * percent),
        .y = v0.y + ((v1.y - v0.y) * percent),
    };
}

void texturedTriangle(FrameBuffer &frameBuffer, Texture2D &texture, Vec3f a, Vec3f b, Vec3f c, Vec2f uv_a, Vec2f uv_b, Vec2f uv_c)
{
    S_RGB color = { 255, 0, 0 };
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;
    auto &buffer = frameBuffer.buffer;

    if (a.y > b.y) {
        std::swap(a, b);
        std::swap(uv_a, uv_b);
    }
    if (a.y > c.y) {
        std::swap(a, c);
        std::swap(uv_a, uv_c);
    }
    if (b.y > c.y) {
        std::swap(b, c);
        std::swap(uv_b, uv_c);
    }


    Vec3f bboxmin = {(float) ( width - 1), (float) (height - 1) };
    Vec3f bboxmax = { 0, 0 };
    Vec3f clamp = {(float) ( width - 1), (float) (height - 1) };

    Vec3f pts[3] = { a, b, c};
    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    float boxHeight = bboxmax.x - bboxmin.x;
    float boxWidth = bboxmax.y - bboxmin.y;

    assert(a.y <= c.y && "deveria ser menor ou igual sempre");

    int totalHeight = c.y -  a.y;
    for (int y = a.y; y <= b.y; y++)
    {
        int segmentHeight = b.y - a.y + 1;
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
        float beta  = (float) (y - a.y) / segmentHeight;

        Vec2i p0 = {
            .x = (int) (a.x + (c.x - a.x) * alpha),
            .y = (int) (a.y + (c.y - a.y) * alpha),
        };

        Vec2i p1 = {
            .x = (int) (a.x + (b.x - a.x) * beta),
            .y = (int) (a.y + (b.y - a.y) * beta),
        };

        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t colorIndex = y * frameBuffer.width + j;
            Vec3f pixelPosition = { .x = (float) p0.x, .y = (float) p0.y, .z = 0 };
            Vec3f bsScreen = barycentric(a, b, c, pixelPosition);

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            pixelPosition.z = 0;

            // calcula posição no exio z do pixel atual
            pixelPosition.z += a.z * bsScreen.x;
            pixelPosition.z += b.z * bsScreen.y;
            pixelPosition.z += c.z * bsScreen.z;

            /**
             * @note João, todo essa secção precisa ser terminada
             * @note João, todo essa secção precisa ser otimizada
             */
            S_RGB sampler2D(Texture2D &texture, float xNormalized, float yNormalized);
            float x = (bboxmax.x - (float) j) / width ;
            float y = (bboxmax.y - (float) p0.y) / height;
            // Vec2f l0 = lerp(uv_a, uv_c, y);
            // Vec2f l1 = lerp(c2, c3, y);
            // Vec2f lr = lerp(l0, l1, y);
            buffer[colorIndex] = sampler2D(texture, x, y);
        }
    }

    for (int y = b.y; y <= c.y; y++)
    {
        int segmentHeight = c.y - b.y + 1;
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
        float beta  = (float) (y - b.y) / segmentHeight;
        
        Vec2i p0 = {
            .x = (int) (a.x + ((c.x - a.x) * alpha)),
            .y = (int) (a.y + ((c.y - a.y) * alpha)),
        };

        Vec2i p1 = {
            .x = (int) (b.x + ((c.x - b.x) * beta)),
            .y = (int) (b.y + ((c.y - b.y) * beta)),
        };


        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t colorIndex = y * frameBuffer.width + j;
            Vec3f pixelPosition = { .x = (float) p0.x, .y = (float) p0.y, .z = 0 };
            Vec3f bsScreen = barycentric(a, b, c, pixelPosition);

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            pixelPosition.z = 0;

            // calcula posição no exio z do pixel atual
            pixelPosition.z += a.z * bsScreen.x;
            pixelPosition.z += b.z * bsScreen.y;
            pixelPosition.z += c.z * bsScreen.z;

            buffer[colorIndex] = color;
        }
    }
}

void triangle(FrameBuffer &frameBuffer, ZBuffer &zBuffer, Vec3f a, Vec3f b, Vec3f c, S_RGB color)
{
    auto &buffer = frameBuffer.buffer;

    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);

    assert(a.y <= c.y && "deveria ser menor ou igual sempre");

    int totalHeight = c.y -  a.y;
    for (int y = a.y; y <= b.y; y++)
    {
        int segmentHeight = b.y - a.y + 1;
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
        float beta  = (float) (y - a.y) / segmentHeight;

        Vec2i p0 = {
            .x = (int) (a.x + (c.x - a.x) * alpha),
            .y = (int) (a.y + (c.y - a.y) * alpha),
        };

        Vec2i p1 = {
            .x = (int) (a.x + (b.x - a.x) * beta),
            .y = (int) (a.y + (b.y - a.y) * beta),
        };

        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t colorIndex = y * frameBuffer.width + j;
            Vec3f pixelPosition = { .x = (float) p0.x, .y = (float) p0.y, .z = 0 };
            Vec3f bsScreen = barycentric(a, b, c, pixelPosition);

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            pixelPosition.z = 0;

            // calcula posição no exio z do pixel atual
            pixelPosition.z += a.z * bsScreen.x;
            pixelPosition.z += b.z * bsScreen.y;
            pixelPosition.z += c.z * bsScreen.z;

            if (zBuffer.buffer[colorIndex] < pixelPosition.z) {
                zBuffer.buffer[colorIndex] = pixelPosition.z;
                buffer[colorIndex] = color;
            }
        }
    }

    for (int y = b.y; y <= c.y; y++)
    {
        int segmentHeight = c.y - b.y + 1;
        float alpha = (float) (y - a.y) / std::max(totalHeight, 1);
        float beta  = (float) (y - b.y) / segmentHeight;
        
        Vec2i p0 = {
            .x = (int) (a.x + ((c.x - a.x) * alpha)),
            .y = (int) (a.y + ((c.y - a.y) * alpha)),
        };

        Vec2i p1 = {
            .x = (int) (b.x + ((c.x - b.x) * beta)),
            .y = (int) (b.y + ((c.y - b.y) * beta)),
        };


        if (p0.x > p1.x) std::swap(p0, p1);

        for (int j = p0.x; j <= p1.x; j++) {
            uint32_t colorIndex = y * frameBuffer.width + j;
            Vec3f pixelPosition = { .x = (float) p0.x, .y = (float) p0.y, .z = 0 };
            Vec3f bsScreen = barycentric(a, b, c, pixelPosition);

            if (bsScreen.x < 0 || bsScreen.y < 0 || bsScreen.z < 0 ) {
                continue;
            };

            if (colorIndex > buffer.size()) {
                continue;
            };

            pixelPosition.z = 0;

            // calcula posição no exio z do pixel atual
            pixelPosition.z += a.z * bsScreen.x;
            pixelPosition.z += b.z * bsScreen.y;
            pixelPosition.z += c.z * bsScreen.z;

            if (zBuffer.buffer[colorIndex] < pixelPosition.z) {
                zBuffer.buffer[colorIndex] = pixelPosition.z;
                buffer[colorIndex] = color;
            }
        }
    }
}

void drawModelWithLightSource(ObjModel* model, FrameBuffer &frameBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    Vec3f lightDir = { 0, 0, -1 };
    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        Vec2i screenCoords[3];
        Vec3f worldCoords[3];
        for (int v = 0; v < 3; v++)
        {
            Vec3f vert = model->getVert(face[v]);
            screenCoords[v].x = (vert.x*140.0) + width/2.0;
            screenCoords[v].y = (vert.y*140.0) + height/4.0;
            worldCoords[v] = vert;
        };
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0)
        {
            triangle2(frameBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB {
                (uint8_t) (intensity * 255),
                (uint8_t) (intensity * 255),
                (uint8_t) (intensity * 255),
            });
        }
    }
}

void drawModelWithLightSource(ObjModel* model, FrameBuffer &frameBuffer, ZBuffer &zBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    assert(frameBuffer.buffer.size() == zBuffer.buffer.size() && "Devem ter o mesmo tamanho");

    Vec3f lightDir = { 0, 0, -1 };
    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];
        for (int v = 0; v < 3; v++)
        {
            Vec3f vert = model->getVert(face[v]);
            screenCoords[v].x = (vert.x*140.0) + width/2.0;
            screenCoords[v].y = (vert.y*140.0) + height/4.0;
            screenCoords[v].z = 0; // @todo João, removendo o eixo z original funciona parcialmente
            worldCoords[v] = vert;
        };
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0)
        {
            triangle(frameBuffer, zBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB {
                (uint8_t) (intensity * 255),
                (uint8_t) (intensity * 255),
                (uint8_t) (intensity * 255),
            });
        }
    }
}

void drawModel(ObjModel* model, FrameBuffer &frameBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        Vec2i verts[3];
        Vec3f vert;
        for (int v = 0; v < 3; v++)
        {
            vert = model->getVert(face[v]);
            verts[v].x = (vert.x*140.0) + width/2.0;
            verts[v].y = (vert.y*140.0) + height/4.0;
        };
        triangle2(frameBuffer, verts[0], verts[1], verts[2], S_RGB { 255, 255, 255 });
        /* triangle2(frameBuffer, verts[0], verts[1], verts[2], S_RGB {
            .r = (uint8_t) (std::rand() % 255),
            .g = (uint8_t) (std::rand() % 255),
            .b = (uint8_t) (std::rand() % 255),
        }); */
    }
}

void drawModel2(ObjModel* model, FrameBuffer &frameBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    Vec3f lightDir = { 0, 0, -1 };
    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        Vec2i screenCoords[3];
        Vec3f worldCoords[3];
        for (int v = 0; v < 3; v++)
        {
            Vec3f vert = model->getVert(face[v]);
            screenCoords[v].x = (vert.x*400.0) + width/2.0;
            screenCoords[v].y = (vert.y*400.0) + height/2.0;
            worldCoords[v] = vert;
        };
        
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0) {
            triangle3(frameBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB { (uint8_t) (intensity * 255), (uint8_t) (intensity * 255), (uint8_t) (intensity * 255) });
            /* triangle3(frameBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB {
                .r = (uint8_t) (std::rand() % 255),
                .g = (uint8_t) (std::rand() % 255),
                .b = (uint8_t) (std::rand() % 255),
            });  */
        }
    }
}

void drawModelWithLightSourceAndZBuffer2(ObjModel* model, FrameBuffer &frameBuffer, ZBuffer &zBuffer)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;

    Vec3f lightDir = { 0, 0, -1 };
    for (int i = 0; i < model->facesLength(); i++)
    {
        auto face = model->getFace(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];
        for (int v = 0; v < 3; v++)
        {
            Vec3f vert = model->getVert(face[v]);
            screenCoords[v].x = (vert.x*400.0) + width/2.0;
            screenCoords[v].y = (vert.y*400.0) + height/2.0;
            screenCoords[v].z = vert.z;
            worldCoords[v] = vert;
        };
        
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0) {
            triangle3(frameBuffer, zBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB { (uint8_t) (intensity * 255), (uint8_t) (intensity * 255), (uint8_t) (intensity * 255) });
            /* triangle3(frameBuffer, screenCoords[0], screenCoords[1], screenCoords[2], S_RGB {
                .r = (uint8_t) (std::rand() % 255),
                .g = (uint8_t) (std::rand() % 255),
                .b = (uint8_t) (std::rand() % 255),
            });  */
        }
    }
}

static inline S_RGB lerp(S_RGB v0, S_RGB v1, float percent)
{
    return S_RGB {
        .r = (uint8_t) (v0.r + ((v1.r - v0.r) * percent)),
        .g = (uint8_t) (v0.g + ((v1.g - v0.g) * percent)),
        .b = (uint8_t) (v0.b + ((v1.b - v0.b) * percent)),
    };
}

/**
 * @brief Primeira implementação que fiz, ela não funciona corretamente, fique com pena de comentar XD
 * 
 * @param texture 
 * @param xNormalized 
 * @param yNormalized 
 * @return S_RGB 
 */
S_RGB sampler2D_antigo(Texture2D &texture, float xNormalized, float yNormalized)
{
    const auto width = texture.width;
    const auto height = texture.height;

    float x = width * xNormalized;
    float y = height * yNormalized;
    int ix = (int) x;
    int iy = (int) y;
    float xDecimal = (x - (long) x);
    float yDecimal = (y - (long) y);

    int horizontal = xDecimal > 0.5 ? 1 : -1;
    int vertical = yDecimal > 0.5 ? 1 : -1;

    int index = iy * width + ix;

    S_RGB c0 = texture.buffer[index];
    if ((ix + horizontal >= 0) && (ix + horizontal < width)) {
        index = iy * width + ix + horizontal;
    }
    S_RGB c1 = texture.buffer[index];
    if ((iy + vertical >= 0) && (iy + vertical < height)) {
        index = iy * width + ix + vertical * width;
    } else {
        index = iy * width + ix;
    }
    S_RGB c2 = texture.buffer[index];
    if (((ix + horizontal >= 0) && (ix + horizontal < width)) && ((iy + vertical >= 0) && (iy + vertical < height))) {
        index = iy * width + ix + horizontal;
        index += vertical * width;
    } else {
        index = iy * width + ix;
    }
    S_RGB c3 = texture.buffer[index];

    // @todo João, implementação incorreta e malfuncionando
    S_RGB l0 = horizontal > 0 ? lerp(c0, c2, xDecimal - 0.5) : lerp(c2, c0, xDecimal + 0.5);
    S_RGB l1 = horizontal > 0 ? lerp(c1, c3, xDecimal - 0.5) : lerp(c3, c1, xDecimal + 0.5);
    S_RGB lr = vertical > 0 ? lerp(l0, l1, xDecimal - 0.5) : lerp(l1, l0, xDecimal + 0.5);

    return lr;
}

S_RGB sampler2D(Texture2D &texture, float xNormalized, float yNormalized)
{
    const auto width = texture.width;
    const auto height = texture.height;

    float x = width * xNormalized;
    float y = height * yNormalized;
    float xDecimal = (x - (long) x);
    float yDecimal = (y - (long) y);

    S_RGB c0 = texture.buffer[ floor(y) * width + floor(x) ];
    S_RGB c1 = texture.buffer[ floor(y) * width + ceil(x) ];
    S_RGB c2 = texture.buffer[ ceil(y) * width + floor(x) ];
    S_RGB c3 = texture.buffer[ ceil(y) * width + ceil(x) ];

    S_RGB l0 = lerp(c0, c1, xDecimal);
    S_RGB l1 = lerp(c2, c3, xDecimal);
    S_RGB lr = lerp(l0, l1, yDecimal);

    return lr;
}

Texture2D downsampleTexture(Texture2D &texture, const unsigned width, const unsigned height)
{
    Texture2D downsampledTexture = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    for (unsigned i = 0; i < height; i++)
    {
        for (unsigned j = 0; j < width; j++)
        {
            auto index = i * width + j;
            auto &color = downsampledTexture.buffer[index];

            // @todo João, avaliar o efeito de adicionar 0.5 ao `i` e `j` antes de multiplicar
            float y = i / (float) height;
            float x = j / (float) width;
            color = sampler2D(texture, x, y);
        }
    }

    return downsampledTexture;
}

void drawTextureToFrame(Texture2D &texture2D, FrameBuffer &frameBuffer, unsigned x, unsigned y, unsigned width, unsigned height)
{
    for (unsigned i = 0; i < height; i++)
    {
        if (i >= texture2D.height) break;

        for (unsigned j = 0; j < width; j++)
        {
            if (j >= texture2D.width) break;

            auto textureColorIndex = i * texture2D.width + j;
            if ((x + j) >= frameBuffer.width) break;
            auto frameBufferColorIndex = (y + i) * frameBuffer.width + (x + j);
            
            if (textureColorIndex >= texture2D.buffer.size()) break;
            if (frameBufferColorIndex >= frameBuffer.buffer.size()) break;

            auto &color = frameBuffer.buffer[frameBufferColorIndex];
            color = texture2D.buffer[textureColorIndex];
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

static inline int lerp(int v0, int v1, float percent)
{
    int diff = v1 - v0;
    return v0 + (diff * percent);
}

void fillLinearGradient(FrameBuffer &frameBuffer, const S_RGB &color0, const S_RGB &color1)
{
    const auto width = frameBuffer.width;
    const auto height = frameBuffer.height;
    auto &buffer = frameBuffer.buffer;
    const auto size = frameBuffer.buffer.size();

    assert(width * height <= size);

    for (int y = 0; y < height; y++)
    {
        S_RGB currentColor;
        for (int x = 0; x < width; x++)
        {
            float percent = ((float) x) / ((float) width);
            currentColor.r = lerp(color0.r, color1.r, percent);
            currentColor.g = lerp(color0.g, color1.g, percent);
            currentColor.b = lerp(color0.b, color1.b, percent);
            uint32_t color_index = y * frameBuffer.width + x;

            if (color_index > size) {
                continue;
            };


            buffer[color_index] = currentColor;
        }

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

void renderHeadWireframeScene()
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

    ObjModel* model = ObjModel::readObjModel2("assets/models/african_head.obj");

    drawRect(frameBuffer, 10, 10, 700, 100, S_RGB { 255, 255, 0 });
    drawRect(frameBuffer, 20, 20, 150, 730, S_RGB { 0, 255, 255 });

    drawWireframe2(model, frameBuffer);

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

void renderTeapotWireframeScene()
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

void renderTeapotWithLightSourceScene()
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


    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });

    ObjModel* model = ObjModel::readObjModel("teapot.obj");

    auto start = std::chrono::high_resolution_clock::now();
    drawModelWithLightSource(model, frameBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tempo renderizando: " << elapsedTime.count() << " ms "<< std::endl;

    flipImageInXAxis(frameBuffer);

    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

void renderTeapotWithLightSourceAndZBufferScene()
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 768;
    const int fov = M_PI / 2.;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    ZBuffer zBuffer(width, height);

    assert(frameBuffer.buffer.size() == zBuffer.buffer.size() && "Devem ter o mesmo tamanho");

    const auto RED = S_RGB { 255, 0, 0 };
    const auto WHITE = S_RGB { 255, 255, 255 };
    const auto BLACK = S_RGB { 0, 0, 0 };


    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });

    ObjModel* model = ObjModel::readObjModel("teapot.obj");

    auto start = std::chrono::high_resolution_clock::now();
    drawModelWithLightSource(model, frameBuffer, zBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tempo renderizando: " << elapsedTime.count() << " ms "<< std::endl;

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}


void renderHeadFilledWithLightSourceAndZBufferScene()
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 1024;
    const int fov = M_PI / 2.;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    ZBuffer zBuffer(width, height);

    assert(frameBuffer.buffer.size() == zBuffer.buffer.size() && "Devem ter o mesmo tamanho");

    const auto RED = S_RGB { 255, 0, 0 };
    const auto WHITE = S_RGB { 255, 255, 255 };
    const auto BLACK = S_RGB { 0, 0, 0 };


    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });

    ObjModel* model = ObjModel::readObjModel2("assets/models/african_head.obj");

    auto start = std::chrono::high_resolution_clock::now();
    drawModelWithLightSourceAndZBuffer2(model, frameBuffer, zBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tempo renderizando: " << elapsedTime.count() << " ms "<< std::endl;

    flipImageInXAxis(frameBuffer);

    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

void renderHeadFilledScene()
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 1024;
    const int fov = M_PI / 2.;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    const auto RED = S_RGB { 255, 0, 0 };
    const auto WHITE = S_RGB { 255, 255, 255 };
    const auto BLACK = S_RGB { 0, 0, 0 };


    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });

    ObjModel* model = ObjModel::readObjModel2("assets/models/african_head.obj");

    auto start = std::chrono::high_resolution_clock::now();
    drawModel2(model, frameBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tempo renderizando: " << elapsedTime.count() << " ms "<< std::endl;

    flipImageInXAxis(frameBuffer);

    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}


void renderTeapotFilledScene()
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


    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });

    ObjModel* model = ObjModel::readObjModel("teapot.obj");

    auto start = std::chrono::high_resolution_clock::now();
    drawModel(model, frameBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tempo renderizando: " << elapsedTime.count() << " ms "<< std::endl;

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

    // fill(frameBuffer, BLACK);
    fillLinearGradient(frameBuffer, S_RGB { 230, 100, 101 }, S_RGB { 145, 152, 229 });
    // fillLinearGradient(frameBuffer, S_RGB { 150, 0, 0 }, S_RGB { 0, 150, 0 });
    // fill(frameBuffer, S_RGB { 145, 152, 229 });

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

void renderProblematicTriangle()
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 768;
    const int fov = M_PI / 2.;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    const auto BLACK = S_RGB { 0, 0, 0 };

    fill(frameBuffer, BLACK);

    Vec2i a = { .x = 719, .y = 566 };
    Vec2i b = { .x = 717, .y = 566 };
    Vec2i c = { .x = 720, .y = 566 };

    // @note: graças a esse triângulo percebe que esqueci de por <= em lugar aonde era necessário
    // também percebi que não tratei a divisão por zero :/
    triangle2(frameBuffer, a, b, c, S_RGB { 255, 255, 255 });

    flipImageInXAxis(frameBuffer);
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

void renderSampledImage()
{
    constexpr unsigned int width = 1024;
    constexpr unsigned int height = 1024;

    FrameBuffer frameBuffer = {
        width : width,
        height : height,
        buffer : vector<S_RGB>(width * height),
    };

    const auto BLACK = S_RGB { 0, 0, 0 };

    Texture2D textureScream = {
        width : 240,
        height : 297,
        buffer : vector<S_RGB>(240 * 297),
    };

    // @todo João, talvez implementar uma função que produza uma versão resized (usando sampling) da textura

    // Por hora carregando manual os dados do array para a textura
    unsigned index = 0;
    for (auto &color : textureScream.buffer)
    {
        color.r = image_scream[index + 0];
        color.g = image_scream[index + 1];
        color.b = image_scream[index + 2];

        index += 4; // tem quatro componentes de um byte (rgba)
    }

    fill(frameBuffer, BLACK);

    drawTextureToFrame(textureScream, frameBuffer, 0, 0, textureScream.width, textureScream.height);

    Texture2D textureScreamDownSampled = downsampleTexture(textureScream, 120, 148);

    drawTextureToFrame(textureScreamDownSampled, frameBuffer, 250, 0, textureScreamDownSampled.width, textureScreamDownSampled.height);

    Texture2D textureScreamUppSampled = downsampleTexture(textureScream, 480, 594);

    drawTextureToFrame(textureScreamUppSampled, frameBuffer, 380, 0, textureScreamUppSampled.width, textureScreamUppSampled.height);

    Texture2D textureScreamStretchedSampled = downsampleTexture(textureScream, 1000, 594);

    drawTextureToFrame(textureScreamStretchedSampled, frameBuffer, 0, 550, textureScreamStretchedSampled.width, textureScreamStretchedSampled.height);

    // @todo João, código de teste do render de texto básico
    {
        Texture2D glyph = createGlyph();
        Texture2D glyph2x = downsampleTexture(glyph, glyph.width * 32, glyph.height * 32);
        drawTextureToFrame(glyph2x, frameBuffer, 0, 550, glyph2x.width, glyph2x.height);
    }

    /**
     * @brief Desenhando um triangulo apenas facilitar a visualização do que seria necessário
     * para adicionar coordenadas UV's.
     * 
     */
    for (float i = 1.0; i < 4.0; i++)
    {
        const float scale = 100.0 * i;
        Vec3f screenCoords[3] = {
            { .x = 0, .y = 1 },
            { .x = 1, .y = -1 },
            { .x = -1, .y = -1 },
        };
        Vec2f uvs[3] = {
            { .x = 0.0, .y = 1.0 },
            { .x = 0.5, .y = 0.0 },
            { .x = 1.0, .y = 1.0 },
        };
        for (int v = 0; v < 3; v++)
        {
            screenCoords[v].x = screenCoords[v].x * scale + width / 4 + scale;
            screenCoords[v].y = screenCoords[v].y * -1 * scale + height / 2; // @note invertendo o eixo y para ficar de acordo com o sentido pretendido
        };
        texturedTriangle(
            frameBuffer,
            textureScream,
            screenCoords[0],
            screenCoords[1],
            screenCoords[2],
            uvs[0],
            uvs[1],
            uvs[2]
        );
    }
    
    saveFrameBufferToPPMFile(frameBuffer, "image.ppm");
}

int main(int argc, char *argv[])
{
    //renderTeapotWireframeScene();
    //renderTriangleTestScene();
    //renderProblematicTriangle();
    //renderTeapotFilledScene();
    //renderTeapotWithLightSourceScene();
    //renderTeapotWithLightSourceAndZBufferScene();
    //renderHeadWireframeScene();
    //renderHeadFilledScene();

    // @todo João, parei aqui
    //renderHeadFilledWithLightSourceAndZBufferScene();

    renderSampledImage();

    return 0;
}

