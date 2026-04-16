#include <algorithm>
#include "our_gl.h"

mat<4,4> ModelView, Viewport, Perspective; // OpenGL状态矩阵
std::vector<double> zbuffer;               // 深度缓冲区

// 构造模型视图矩阵（相机变换）
// 参数：eye(摄像机位置), center(看向的中心点), up(摄像机向上方向)
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);  // 摄像机前向量（指向摄像机）
    vec3 l = normalized(cross(up,n)); // 摄像机右向量
    vec3 m = normalized(cross(n, l)); // 摄像机实际的上向量
    // 将世界坐标变换到摄像机坐标系（旋转+平移）
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

// 初始化透视投影矩阵
// 参数：f(摄像机到近平面的距离)
void init_perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

// 初始化视口变换矩阵（从标准化设备坐标到屏幕坐标）
// 参数：x(视口左上角x), y(视口左上角y), w(视口宽度), h(视口高度)
void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

// 初始化深度缓冲区（用于深度比较和可见性判断）
// 参数：width(屏幕宽度), height(屏幕高度)
void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector<double>(width*height, -1000.);
}

// 光栅化函数：将三角形绘制到帧缓冲中
// 参数：clip(裁剪空间中的三角形), shader(着色器), framebuffer(帧缓冲)
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer) {
    // 进行透视除法，从裁剪空间转换到标准化设备坐标
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };                // 标准化设备坐标
    // 使用视口变换将标准化设备坐标转换到屏幕坐标
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() }; // 屏幕坐标

    // 构建用于重心坐标计算的矩阵
    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if (ABC.det()<1) return; // 背面剔除 + 丢弃覆盖面积小于一像素的三角形

    // 计算三角形的包围盒
    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // 三角形的包围盒
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // 由其左上角和右下角定义
    
#pragma omp parallel for
    // 遍历包围盒内的所有像素
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) {         // 通过屏幕边界裁剪包围盒
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            // 计算像素相对于三角形的重心坐标
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // 重心坐标
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;                                                    // 负重心坐标 => 像素在三角形外
            // 使用重心坐标对深度进行线性插值
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };  // 深度的线性插值
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;   // 丢弃深度超过z缓冲区的片段
            // 调用片段着色器计算像素的最终颜色
            auto [discard, color] = shader.fragment(bc);
            if (discard) continue;                                 // 片段着色器可以丢弃当前片段
            zbuffer[x+y*framebuffer.width()] = z;                  // 更新z缓冲区
            framebuffer.set(x, y, color);                          // 更新帧缓冲区
        }
    }
}