#include <cstdlib>
#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // OpenGL状态矩阵：模型视图矩阵和透视投影矩阵
extern std::vector<double> zbuffer;     // 深度缓冲区

// Phong光照着色器
struct PhongShader : IShader {
    const Model &model;
    TGAColor color = {};      // 表面颜色
    vec3 tri[3];              // 眼坐标空间中三角形的三个顶点坐标

    // 构造函数：初始化着色器
    PhongShader(const Model &m) : model(m) {
    }

    // 顶点着色器：将物体空间的顶点变换到眼坐标和裁剪空间
    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // 获取物体坐标空间中的顶点
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};   // 通过模型视图矩阵变换到眼坐标空间
        tri[vert] = gl_Position.xyz();                            // 保存眼坐标下的顶点坐标
        return Perspective * gl_Position;                         // 通过透视投影矩阵返回裁剪空间的坐标
    }

    // 片段着色器：使用法线计算Phong光照模型
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        // 初始化片段颜色为白色
        TGAColor gl_FragColor = {255, 255, 255, 255};
        
        // 获取光照参数
        vec3 l = normalized(vec3{1, 1, 1}); // 光源方向（标准化）
        vec3 n = normalized(cross(tri[1]-tri[0], tri[2]-tri[0])); // 几何法线（由三角形两条边的叉积获得）
        vec3 r = normalized(l + vec3{0, 0, 1});  // 半程向量（用于镜面反射计算）

        // 计算Phong光照模型的各个分量
        double ambient = 0.3;                           // 环境光强度
        double diffuse = std::max(0., n*l);             // 漫反射强度（法线与光线方向的点积）
        double specular = std::pow(std::max(0., r * n), 40); // 镜面反射强度（指数为40）
        
        // 将光照效果应用到每个颜色通道（RGB）
        for (int channel : {0,1,2})
            gl_FragColor[channel] *= std::min(1., ambient + .4*diffuse + .9*specular);
        return {false, gl_FragColor};
    }
};

int main(int argc, char** argv) {
    // 检查命令行参数
    if (argc < 2) {
        std::cerr << "使用方法: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // 输出图像宽度
    constexpr int height = 800;      // 输出图像高度
    constexpr vec3    eye{-1, 0, 2}; // 摄像机位置
    constexpr vec3 center{ 0, 0, 0}; // 摄像机朝向（观察点）
    constexpr vec3     up{ 0, 1, 0}; // 摄像机上方向

    // 初始化各个变换矩阵
    lookat(eye, center, up);                                   // 构建模型视图矩阵
    init_perspective(norm(eye-center));                        // 构建透视投影矩阵
    init_viewport(width/16, height/16, width*7/8, height*7/8); // 构建视口变换矩阵
    init_zbuffer(width, height);                               // 初始化深度缓冲区
    TGAImage framebuffer(width, height, TGAImage::RGB, {0, 0, 0, 255}); // 创建帧缓冲（黑色背景）

    // 遍历所有输入的模型文件
    for (int m=1; m<argc; m++) {
        Model model(argv[m]);                       // 加载OBJ模型数据
        PhongShader shader(model);                  // 为模型创建Phong光照着色器
        
        // 遍历模型的所有三角形面
        for (int f=0; f<model.nfaces(); f++) {
            // 为每个面设置随机颜色
            shader.color = { 
                static_cast<std::uint8_t>(std::rand() % 255), 
                static_cast<std::uint8_t>(std::rand() % 255), 
                static_cast<std::uint8_t>(std::rand() % 255), 
                255 
            };
            // 组装三角形图元（三个顶点的同次坐标）
            Triangle clip = { shader.vertex(f, 0),
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            // 光栅化三角形并写入帧缓冲
            rasterize(clip, shader, framebuffer);
        }
    }

    // 将帧缓冲写入TGA图像文件
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}