#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
    std::vector<vec3> verts_;               // 存储所有顶点的坐标
    std::vector<std::vector<int> > faces_;  // 存储每个面包含的顶点索引

public:
    // 构造函数和析构函数
    Model(const char *filename);  // 从OBJ文件加载模型数据
    ~Model();                     // 析构函数
    
    // 获取顶点相关信息的方法
    int nverts() const;                      // 获取顶点总数
    int nfaces() const;                      // 获取面的总数
    vec3 vert(int i) const;                  // 根据全局索引获取顶点坐标
    vec3 vert(int iface, int nthvert) const; // 根据面索引和面内顶点索引获取顶点坐标
    
    // 获取面信息的方法
    std::vector<int> face(int idx);  // 获取指定面的顶点索引列表
};

#endif //__MODEL_H__