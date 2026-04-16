#include <fstream>
#include <sstream>
#include "model.h"

// 模型构造函数：从 OBJ 文件加载模型数据和纹理
Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    // 逐行读取 OBJ 文件
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            // 解析顶点数据
            iss >> trash;
            vec4 v = {0,0,0,1};
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            // 解析法向量数据
            iss >> trash >> trash;
            vec4 n;
            for (int i : {0,1,2}) iss >> n[i];
            norms.push_back(normalized(n));
        } else if (!line.compare(0, 3, "vt ")) {
            // 解析纹理坐标数据
            iss >> trash >> trash;
            vec2 uv;
            for (int i : {0,1}) iss >> uv[i];
            tex.push_back({uv.x, 1-uv.y});  // 翻转 V 坐标
        } else if (!line.compare(0, 2, "f ")) {
            // 解析面数据（顶点/纹理坐标/法向量的索引）
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);   // 顶点索引（OBJ 中从 1 开始，转换为 0 开始）
                facet_tex.push_back(--t);   // 纹理坐标索引
                facet_nrm.push_back(--n);   // 法向量索引
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "错误：OBJ 文件中的面必须是三角形" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# 顶点数: " << nverts() << " 面数: "  << nfaces() << std::endl;
    
    // 纹理加载 Lambda 函数
    auto load_texture = [&filename](const std::string suffix, TGAImage &img) {
        size_t dot = filename.find_last_of(".");
        if (dot==std::string::npos) return;
        std::string texfile = filename.substr(0,dot) + suffix;
        std::cerr << "纹理文件 " << texfile << " 加载 " << (img.read_tga_file(texfile.c_str()) ? "成功" : "失败") << std::endl;
    };
    
    // 加载各类纹理
    load_texture("_diffuse.tga",    diffusemap );   // 漫反射纹理
    load_texture("_nm_tangent.tga", normalmap);     // 切线空间法线贴图
    load_texture("_spec.tga",       specularmap);   // 高光纹理
}

// 获取顶点数
int Model::nverts() const { return verts.size(); }
// 获取三角形面数
int Model::nfaces() const { return facet_vrt.size()/3; }

// 获取索引为 i 的顶点（0 <= i < nverts()）
vec4 Model::vert(const int i) const {
    return verts[i];
}

// 获取第 iface 个三角形的第 nthvert 个顶点
vec4 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

// 获取第 iface 个三角形的第 nthvert 个顶点的法向量
vec4 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_nrm[iface*3+nthvert]];
}

// 从法线贴图中采样切线空间的法向量
vec4 Model::normal(const vec2 &uv) const {
    TGAColor c = normalmap.get(uv[0]*normalmap.width(), uv[1]*normalmap.height());
    // 将 RGB 颜色值转换为 [-1, 1] 范围的法向量
    return normalized(vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0});
}

// 获取第 iface 个三角形的第 nthvert 个顶点的纹理坐标
vec2 Model::uv(const int iface, const int nthvert) const {
    return tex[facet_tex[iface*3+nthvert]];
}

// 获取漫反射纹理
const TGAImage& Model::diffuse()  const { return diffusemap;  }
// 获取高光纹理
const TGAImage& Model::specular() const { return specularmap; }