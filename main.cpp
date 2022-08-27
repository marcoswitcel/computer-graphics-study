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

    saveFrameBuffertoPPMFile(frameBuffer, "image.ppm");

    return 0;
}

