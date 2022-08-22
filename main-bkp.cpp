#include <iostream>
#include <fstream>

using namespace std;


void writePPM(const char *filename)
{
    constexpr auto dimx = 800u, dimy = 800u;
    ofstream ofs(filename, ios_base::out | ios_base::binary);

    ofs << "P6" << endl
        << dimx << ' ' << dimy << endl
        << "255" << endl;

    for (auto j = 0u; j < dimy; ++j)
    {
        for (auto i = 0u; i < dimx; ++i)
        {
            ofs << (char)(i % 256) << (char)(j % 256) << (char)((i * j) % 256); // red, green, blue
        }
    }

    ofs.close();
}

template <typename Type>
Type sum(Type a, Type b)
{
    return a + b;
}

int main(int argc, char *argv[])
{
    cout << "testando" << sum<float>(1,2);
    writePPM("teste.ppm");

    return 0;
}

