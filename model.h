#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
    std::vector<vec3> verts_;               // 存所有顶点的坐标
    std::vector<std::vector<int> > faces_;  // 存每个面包含的顶点编号

public:
    Model(const char *filename);
    ~Model();
   // 在 model.h 的 public 部分：
    int nverts() const;
    int nfaces() const;
    vec3 vert(int i) const;
    vec3 vert(int iface, int nthvert) const;
        
    std::vector<int> face(int idx);
};

#endif //__MODEL_H__