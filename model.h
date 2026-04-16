#include <vector>
#include "geometry.h"

class Model {
    std::vector<vec3> verts = {};    // 顶点坐标数组
    std::vector<vec3> norms = {};    // 顶点法线向量数组
    std::vector<int> facet_vrt = {}; // 每个三角形顶点的索引
    std::vector<int> facet_nrm = {}; // 每个三角形法线的索引
public:
    // 构造函数
    Model(const std::string filename);
    
    // 获取顶点和面的数量
    int nverts() const; // 顶点数
    int nfaces() const; // 三角形面数
    
    // 获取顶点相关信息
    vec3 vert(const int i) const;                          // 根据全局索引获取顶点坐标
    vec3 vert(const int iface, const int nthvert) const;   // 根据面索引和面内顶点编号获取顶点坐标
    
    // 获取法线相关信息
    vec3 normal(const int iface, const int nthvert) const; // 根据面索引和面内顶点编号获取法线向量
};