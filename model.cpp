#include <fstream>
#include <sstream>
#include "model.h"

// 根据 文件路径加载模型
Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        // 处理顶点（v 命令）
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec4 v = {0,0,0,1};
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        }
        // 处理法线（vn 命令）
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec4 n;
            for (int i : {0,1,2}) iss >> n[i];
            norms.push_back(normalized(n));
        }
        // 处理纹理坐标（vt 命令）
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i : {0,1}) iss >> uv[i];
            // 穻坐标 Y 经常需要翻转（OBJ 格式的 UV 系统常常是翻转的）
            tex.push_back({uv.x, 1-uv.y});
        }
        // 处理面片（f 命令）
        else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            // 检查是否是三角形（OBJ 文件需要是三角流化的）
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;
    // 加载纹理映射文本文件
    auto load_texture = [&filename](const std::string suffix, TGAImage &img) {
        size_t dot = filename.find_last_of(".");
        if (dot==std::string::npos) return;
        std::string texfile = filename.substr(0,dot) + suffix;
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    };
    // 加载漫反射纹理（颜色）
    load_texture("_diffuse.tga",    diffusemap );
//  load_texture("_glow.tga",    diffusemap );
    // 加载法线贴图、高光纹理
    load_texture("_nm_tangent.tga", normalmap);
    load_texture("_spec.tga",       specularmap);
}

// 获取顶点数量
int Model::nverts() const { return verts.size(); }
// 获取三角形数量
int Model::nfaces() const { return facet_vrt.size()/3; }

// 获取第 i 个顶点
vec4 Model::vert(const int i) const {
    return verts[i];
}

// 获取第 iface 个三角形的第 nthvert 个顶点
vec4 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

// 获取第 iface 个三角形的第 nthvert 个顶点法线
vec4 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_nrm[iface*3+nthvert]];
}

// 根据 UV 坐标从法线贴图中获取法线向量
vec4 Model::normal(const vec2 &uv) const {
    TGAColor c = normalmap.get(uv[0]*normalmap.width(), uv[1]*normalmap.height());
    return normalized(vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0});
}

// 获取第 iface 个三角形的第 nthvert 个顶点的 UV 坐标
vec2 Model::uv(const int iface, const int nthvert) const {
    return tex[facet_tex[iface*3+nthvert]];
}

// 获取漫反射纹理
const TGAImage& Model::diffuse()  const { return diffusemap;  }
// 获取高光纹理
const TGAImage& Model::specular() const { return specularmap; }