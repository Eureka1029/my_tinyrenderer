#include "tgaimage.h"
#include "geometry.h"

// 构建模型视图矩阵（摄像机变换）
void lookat(const vec3 eye, const vec3 center, const vec3 up);
// 初始化透视投影矩阵
void init_perspective(const double f);
// 初始化视口变换（屏幕映射）
void init_viewport(const int x, const int y, const int w, const int h);
// 初始化深度缓冲区
void init_zbuffer(const int width, const int height);

// 着色器接口基类
struct IShader {
    // 片段着色器虚函数：返回是否丢弃该像素，以及输出颜色
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

// 三角形图元类型：由三个有序点（同次坐标）组成
typedef vec4 Triangle[3];
// 光栅化函数：将三角形绘制到帧缓冲中
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);