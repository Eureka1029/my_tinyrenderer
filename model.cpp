#include <iostream>  // 输入输出流
#include <fstream>   // 文件流
#include <sstream>   // 字符串流
#include "model.h"

// 构造函数：从OBJ文件中加载模型数据（顶点坐标、顶点法线、面索引信息）
Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;  // 文件打开失败则直接返回
    
    std::string line;
    while (!in.eof()) {  // 逐行读取文件
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        
        if (!line.compare(0, 2, "v ")) {
            // 读取顶点坐标行（v x y z格式）
            iss >> trash;  // 跳过'v'字符
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];  // 读取x、y、z三个坐标
            verts.push_back(v);  // 保存到顶点数组
            
        } else if (!line.compare(0, 3, "vn ")) {
            // 读取顶点法线行（vn x y z格式）
            iss >> trash >> trash;  // 跳过'v'和'n'字符
            vec3 n;
            for (int i : {0,1,2}) iss >> n[i];  // 读取法线的x、y、z分量
            norms.push_back(normalized(n));  // 保存归一化后的法线到法线数组
            
        } else if (!line.compare(0, 2, "f ")) {
            // 读取面索引行（f v/vt/vn v/vt/vn v/vt/vn格式）
            int f,t,n, cnt = 0;  // f:顶点索引, t:纹理坐标索引, n:法线索引, cnt:顶点计数
            iss >> trash;  // 跳过'f'字符
            
            // 循环读取该面的所有顶点索引
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);   // 保存顶点索引（OBJ索引从1开始，转换为0开始）
                facet_nrm.push_back(--n);   // 保存法线索引（同样转换为0开始）
                cnt++;
            }
            
            // 检查是否为三角形（一个面应该有3个顶点）
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    
    // 输出加载完成的统计信息
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;
}

// 获取模型中顶点的总数
int Model::nverts() const { 
    return verts.size(); 
}

// 获取模型中三角形面的总数
int Model::nfaces() const { 
    return facet_vrt.size()/3;  // facet_vrt存储所有三角形的顶点索引，每个三角形3个顶点
}

// 根据全局索引获取顶点坐标
vec3 Model::vert(const int i) const {
    return verts[i];
}

// 根据面索引和面内顶点编号获取顶点坐标
// iface：三角形面的索引，nthvert：该面内的顶点编号（0、1或2）
vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

// 根据面索引和面内顶点编号获取顶点的法线向量
// iface：三角形面的索引，nthvert：该面内的顶点编号（0、1或2）
vec3 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_nrm[iface*3+nthvert]];
}