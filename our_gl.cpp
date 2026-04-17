#include <algorithm>
#include "our_gl.h"

// OpenGL 状态矩阵：模型视图、视口、透视投影
mat<4,4> ModelView, Viewport, Perspective;
// 深度缓冲区，用于深度测试
std::vector<double> zbuffer;

// 设置观察矩阵（相机位置、观看方向和上向量）
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    // 计算相机坐标系的三个基向量
    vec3 n = normalized(eye-center);      // 相机前方向量
    vec3 l = normalized(cross(up,n));     // 相机右向量
    vec3 m = normalized(cross(n, l));     // 相机上向量
    // ModelView = 旋转矩阵 * 平移矩阵
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-eye.x}, {0,1,0,-eye.y}, {0,0,1,-eye.z}, {0,0,0,1}}};
}

// 初始化透视投影矩阵（f 是焦距）
void init_perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

// 初始化视口矩阵（将标准化设备坐标转换到屏幕坐标）
void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

// 初始化深度缓冲区
void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector<double>(width*height, -1000.);
}

// 光栅化三角形：将裁剪坐标中的三角形转换为屏幕像素
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer) {
    // 将齐次坐标转换为标准化设备坐标（NDC）
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };
    // 使用视口矩阵将 NDC 转换为屏幕坐标
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() };

    // 将三个顶点组成矩阵用于重心坐标计算
    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    // 背面剔除 + 剔除面积小于一个像素的三角形
    if (ABC.det()<1) return;

    // 计算三角形的外包矩形框（左上角和右下角）
    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y});
    // 使用 OpenMP 并行处理每个像素
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) {
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            // 计算像素点 {x,y} 相对于三角形的重心坐标（屏幕空间）
            vec3 bc_screen = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            // 转换为裁剪空间的重心坐标，处理透视变形问题
            vec3 bc_clip   = { bc_screen.x/clip[0].w, bc_screen.y/clip[1].w, bc_screen.z/clip[2].w };
            // 归一化权重
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            // 屏幕空间重心坐标为负表示像素在三角形外部
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            // 线性插值计算像素的深度值
            double z = bc_screen * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            // 深度测试：丢弃深度值不符合条件的片段
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            // 调用片段着色器
            auto [discard, color] = shader.fragment(bc_clip);
            // 片段着色器可以丢弃当前片段
            if (discard) continue;
            // 更新深度缓冲区
            zbuffer[x+y*framebuffer.width()] = z;
            // 更新帧缓冲区
            framebuffer.set(x, y, color);
        }
    }
}