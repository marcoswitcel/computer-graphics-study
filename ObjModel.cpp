#ifndef OBJ_MODEL_CPP
#define OBJ_MODEL_CPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "geometry.cpp"


class ObjModel {
public:
    std::vector<Vec3f> verts;
    std::vector<std::vector<int>> faces;
    ObjModel(): verts(), faces() {};
    int vertsLength() { return (int) verts.size(); };
    int facesLength() { return (int) faces.size(); };
    Vec3f getVert(int index) { return verts[index]; };
    std::vector<int> getFace(int index) { return faces[index]; };

    static ObjModel* readObjModel(const char *filename);
};


ObjModel* ObjModel::readObjModel(const char *filename) {
    std::ifstream in(filename, std::ifstream::in);

    if (in.fail()) {
        std::cerr << "probleminha carregando arquivo : \"" << filename << "\"";
        return NULL;
    }
    ObjModel* objModel = new ObjModel;

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            std::cout << "vértice " << line << std::endl;
            Vec3f vec = {0};
            iss >> trash;
            iss >> vec.x;
            iss >> vec.y;
            iss >> vec.z;
            objModel->verts.push_back(vec);
        } else if (!line.compare(0, 2, "f ")) {
            std::cout << "face " << line << std::endl;
        }
    }

    in.close();

    return objModel;
}

#endif // OBJ_MODEL_CPP
