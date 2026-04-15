#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            float x, y, z;
            iss >> x >> y >> z;
            verts_.push_back({x, y, z}); 
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            // 解析 v/vt/vn 格式，提取前面的顶点索引
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // .obj 索引从 1 开始，C++ 从 0 开始，所以减 1
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() const {
    return (int)verts_.size();
}

int Model::nfaces() const {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

vec3 Model::vert(int i) const {
    return verts_[i];
}

// 👇 快捷方法的具体实现：先查 faces_ 拿到顶点编号，再去 verts_ 拿实际坐标
vec3 Model::vert(int iface, int nthvert) const {
    return verts_[faces_[iface][nthvert]];
}