#include "tgaimage.h"
#include "geometry.h"

// 设置观察矩阵
void lookat(const vec3 eye, const vec3 center, const vec3 up);
// 设置透视投影矩阵
void init_perspective(const double f);
// 设置视口矩阵
void init_viewport(const int x, const int y, const int w, const int h);
// 初始化深度缓冲区
void init_zbuffer(const int width, const int height);

// 着色器接口
struct IShader {
    // 从纹理中采样（双线性插值）
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }
    // 片段着色器，返回是否丢弃该片段和颜色值
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

// 三角形原始体: 由三个有序点组成
typedef vec4 Triangle[3];
// 光栅化三角形
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);