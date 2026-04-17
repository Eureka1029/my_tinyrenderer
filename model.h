#include "geometry.h"
#include "tgaimage.h"

// 模型类，用于加载 OBJ 文件中的模型数据
class Model {
    // 顶点推导数组
    std::vector<vec4> verts = {};
    // 法线向量推导数组
    std::vector<vec4> norms = {};
    // 纹理坐标推导数组
    std::vector<vec2> tex = {};
    // 上述数组中的每个三角形索引（大小等于 nfaces()*3）
    std::vector<int> facet_vrt = {};
    std::vector<int> facet_nrm = {};
    std::vector<int> facet_tex = {};
    // 漫反射纹理（体贴颜色）
    TGAImage diffusemap  = {};
    // 法线贴图
    TGAImage normalmap   = {};
    // 高光纹理
    TGAImage specularmap = {};
public:
    // 根据 OBJ 文件路径构造模型
    Model(const std::string filename);
    // 获取顶点数量
    int nverts() const;
    // 获取三角形数量
    int nfaces() const;
    // 获取第 i 个顶点（i >= 0 && i < nverts()）
    vec4 vert(const int i) const;
    // 获取第 iface 个三角形的第 nthvert 个顶点
    vec4 vert(const int iface, const int nthvert) const;
    // 获取第 iface 个三角形的第 nthvert 个顶点法线（来自 OBJ 文件中的 vn 行）
    vec4 normal(const int iface, const int nthvert) const;
    // 根据 UV 坐标从法线贴图中获取法线向量
    vec4 normal(const vec2 &uv) const;
    // 获取第 iface 个三角形的第 nthvert 个顶点的 UV 坐标
    vec2 uv(const int iface, const int nthvert) const;
    // 获取漫反射纹理
    const TGAImage& diffuse() const;
    // 获取高光纹理
    const TGAImage& specular() const;

};