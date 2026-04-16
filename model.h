#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec4> verts = {};    // 顶点数组          ┐ 一般来说，这些数组
    std::vector<vec4> norms = {};    // 法向量数组        │ 的大小不同
    std::vector<vec2> tex = {};      // 纹理坐标数组      ┘ 详见 Model() 构造函数日志
    std::vector<int> facet_vrt = {}; //  ┐ 每个三角形在上述数组中的索引
    std::vector<int> facet_nrm = {}; //  │ 大小应为 nfaces()*3
    std::vector<int> facet_tex = {}; //  ┘
    TGAImage normalmap   = {};       // 切线空间法线贴图纹理（_nm_tangent.tga）
    TGAImage diffusemap  = {};       // 漫反射纹理（_diffuse.tga）
    TGAImage specularmap = {};       // 高光纹理（_spec.tga）
public:
    Model(const std::string filename);
    int nverts() const; // 顶点数
    int nfaces() const; // 三角形数
    vec4 vert(const int i) const;                          // 0 <= i < nverts()
    vec4 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec4 normal(const int iface, const int nthvert) const; // 来自 .obj 文件中 "vn x y z" 的法向量
    vec4 normal(const vec2 &uv) const;                     // 从法线贴图纹理获取法向量
    const TGAImage& diffuse()  const;                      // 获取漫反射纹理
    const TGAImage& specular() const;                      // 获取高光纹理
    vec2 uv(const int iface, const int nthvert) const;     // 三角形顶点的纹理坐标
};
