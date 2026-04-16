#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // OpenGL状态矩阵：模型视图矩阵和透视投影矩阵
extern std::vector<double> zbuffer;     // 深度缓冲区

// Phong光照着色器
struct PhongShader : IShader {
    const Model &model;
    vec3 l;          // 眼坐标空间中的光源方向
    vec3 tri[3];     // 眼坐标空间中的三角形顶点
    vec3 varying_nrm[3]; // 每个顶点的法线向量，用于片段着色器插值

    // 构造函数：初始化着色器并將光源方向转换到眼坐标空间
    PhongShader(const vec3 light, const Model &m) : model(m) {
        // 将光源方向向量变换到视图坐标空间
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}).xyz());
    }

    // 顶点着色器：变换顶点和法线到眼坐标和裁剪空间
    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // 获取物体坐标空间中的顶点
        vec3 n = model.normal(face, vert);                        // 获取顶点法线
        // 使用逆转置矩阵将法线变换到眼坐标空间（保持法线正确性）
        varying_nrm[vert] = (ModelView.invert_transpose() * vec4{n.x, n.y, n.z, 0.}).xyz();
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};   // 变换到眼坐标空间
        tri[vert] = gl_Position.xyz();                            // 保存眼坐标下的顶点
        return Perspective * gl_Position;                         // 返回裁剪空间坐标
    }

    // 片段着色器：使用插值法线计算Phong光照
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = {255, 255, 255, 255};             // 片段输出颜色
        // 使用重心坐标对三个顶点的法线进行插值
        vec3 n = normalized(varying_nrm[0] * bar[0] +
                            varying_nrm[1] * bar[1] +
                            varying_nrm[2] * bar[2]);             // 逐顶点法线插值
        vec3 r = normalized(n * (n * l)*2 - l);                   // 计算反射光方向
        
        // 计算Phong光照的各个分量
        double ambient = .3;                                      // 环境光强度
        double diff = std::max(0., n * l);                        // 漫反射强度
        // 镜面反射强度：摄像机位于z轴上（眼空间坐标），所以简单使用r.z
        double spec = std::pow(std::max(r.z, 0.), 35);            
        
        // 将光照效果应用到每个颜色通道（RGB）
        for (int channel : {0,1,2})
            gl_FragColor[channel] *= std::min(1., ambient + .4*diff + .9*spec);
        return {false, gl_FragColor};                             // 不丢弃该像素
    }
};

int main(int argc, char** argv) {
    // 检查命令行参数
    if (argc < 2) {
        std::cerr << "使用方法: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    // 设置渲染参数
    constexpr int width  = 800;      // 输出图像宽度
    constexpr int height = 800;      // 输出图像高度
    constexpr vec3  light{ 1, 1, 1}; // 光源位置
    constexpr vec3    eye{-1, 0, 2}; // 摄像机位置
    constexpr vec3 center{ 0, 0, 0}; // 摄像机指向的中心点
    constexpr vec3     up{ 0, 1, 0}; // 摄像机上方向

    // 初始化各个变换矩阵
    lookat(eye, center, up);                                   // 构建模型视图矩阵
    init_perspective(norm(eye-center));                        // 构建透视投影矩阵
    init_viewport(width/16, height/16, width*7/8, height*7/8); // 构建视口变换矩阵
    init_zbuffer(width, height);                               // 初始化深度缓冲区
    TGAImage framebuffer(width, height, TGAImage::RGB);        // 创建帧缓冲

    // 遍历所有输入的模型文件
    for (int m=1; m<argc; m++) {                    // 遍历所有输入的对象
        Model model(argv[m]);                       // 加载模型数据
        PhongShader shader(light, model);           // 为模型创建Phong光照着色器
        
        // 遍历模型的所有三角形面
        for (int f=0; f<model.nfaces(); f++) {      // 遍历所有面
            // 组装三角形图元（三个顶点的同次坐标）
            Triangle clip = { shader.vertex(f, 0),  // 组装图元
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // 光栅化该三角形到帧缓冲
        }
    }

    // 将帧缓冲写入TGA图像文件
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}