#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

// 从OBJ文件中加载模型数据（顶点坐标和面索引）
Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "错误：无法打开文件 " << filename << std::endl;
        return;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            // 读取顶点坐标行（v x y z格式）
            iss >> trash;
            float x, y, z;
            iss >> x >> y >> z;
            verts_.push_back({x, y, z}); 
        } else if (!line.compare(0, 2, "f ")) {
            // 读取面索引行（f v/vt/vn v/vt/vn v/vt/vn格式）
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            // 解析 v/vt/vn 格式，提取顶点索引（v）部分
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // OBJ索引从1开始，C++从0开始，所以需要减1转换
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# 顶点数: " << verts_.size() << " 面数: "  << faces_.size() << std::endl;
}

// 析构函数（清理资源）
Model::~Model() {
}

// 获取模型中顶点的总数
int Model::nverts() const {
    return (int)verts_.size();
}

// 获取模型中面的总数
int Model::nfaces() const {
    return (int)faces_.size();
}

// 获取指定面的所有顶点索引
std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

// 根据全局索引获取顶点的三维坐标
vec3 Model::vert(int i) const {
    return verts_[i];
}

// 根据面索引和面内顶点编号获取顶点坐标
// 先通过faces_[iface][nthvert]获取顶点的全局索引，再从verts_中取出实际坐标
vec3 Model::vert(int iface, int nthvert) const {
    return verts_[faces_[iface][nthvert]];
}