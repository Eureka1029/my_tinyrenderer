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
    // 从纹理中采样颜色值
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        const double u = std::max(0.0, std::min(uvf[0], 1.0));
        const double v = std::max(0.0, std::min(uvf[1], 1.0));

        const double x = u * (img.width()  - 1);
        const double y = v * (img.height() - 1);

        const int x0 = static_cast<int>(std::floor(x));
        const int y0 = static_cast<int>(std::floor(y));
        const int x1 = std::min(x0 + 1, img.width()  - 1);
        const int y1 = std::min(y0 + 1, img.height() - 1);

        const double s = x - x0;
        const double t = y - y0;

        const TGAColor c00 = img.get(x0, y0);
        const TGAColor c10 = img.get(x1, y0);
        const TGAColor c01 = img.get(x0, y1);
        const TGAColor c11 = img.get(x1, y1);

        TGAColor out{};
        out.bytespp = c00.bytespp;
        for (int ch = 0; ch < 4; ++ch) {
            const double c0 = c00[ch] * (1.0 - s) + c10[ch] * s;
            const double c1 = c01[ch] * (1.0 - s) + c11[ch] * s;
            const double c  = c0 * (1.0 - t) + c1 * t;
            out[ch] = static_cast<unsigned char>(std::max(0.0, std::min(c, 255.0)));
        }
        return out;
    }
        // 片段着色器，返回是否丢弃该片段和颜色值
        virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
    };

// 三角形原始体: 由三个有序点组成
typedef vec4 Triangle[3];
// 光栅化三角形
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);