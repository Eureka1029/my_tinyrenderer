#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // OpenGL 状态矩阵
extern std::vector<double> zbuffer;     // 深度缓冲区

struct PhongShader : IShader {
    const Model &model;
    vec4 l;              // 眼坐标系中的光线方向
    vec2  varying_uv[3]; // 三角形纹理坐标，由顶点着色器写入，片段着色器读取
    vec4 varying_nrm[3]; // 每个顶点的法向量，由片段着色器插值
    vec4 tri[3];         // 眼坐标系中的三角形

    PhongShader(const vec3 light, const Model &m) : model(m) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.})); // 将光向量变换到眼坐标系
    }

    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert]  = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position;
        return Perspective * gl_Position;                         // 返回裁剪空间坐标
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        // 计算 Darboux 坐标系（切线、副切线、法线）
        mat<2,4> E = { tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U = { varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        mat<4,4> D = {normalized(T[0]),  // 切线向量
                      normalized(T[1]),  // 副切线向量
                      normalized(varying_nrm[0]*bar[0] + varying_nrm[1]*bar[1] + varying_nrm[2]*bar[2]), // 插值后的法向量
                      {0,0,0,1}}; // Darboux 标准正交基
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        // 将切线空间的法线转换到眼坐标系
        vec4 n = normalized(D.transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l)*2 - l);                   // 反射光线方向
        double ambient  = .4;                                     // 环境光强度
        double diffuse  = 1.*std::max(0., n * l);                 // 漫反射强度
        double specular = (3.*sample2D(model.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);  // 高光强度，摄像机在 z 轴上（眼坐标中）
        TGAColor gl_FragColor = sample2D(model.diffuse(), uv);
        // 应用 Phong 光照模型
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        return {false, gl_FragColor};                             // 不丢弃像素
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        // 默认加载 Diablo III 模型
        const char* default_model = "obj/diablo3_pose/diablo3_pose.obj";
        argv[1] = (char*)default_model;
        argc = 2;
    }

    constexpr int width  = 800;      // 输出图像宽度
    constexpr int height = 800;      // 输出图像高度
    constexpr vec3  light{ 1, 1, 1}; // 光源位置
    constexpr vec3    eye{-1, 0, 2}; // 摄像机位置
    constexpr vec3 center{ 0, 0, 0}; // 摄像机看向的中心
    constexpr vec3     up{ 0, 1, 0}; // 摄像机向上向量

    lookat(eye, center, up);                                   // 构建模型视图矩阵
    init_perspective(norm(eye-center));                        // 构建透视投影矩阵
    init_viewport(width/16, height/16, width*7/8, height*7/8); // 构建视口变换矩阵
    init_zbuffer(width, height);                               // 初始化深度缓冲区
    TGAImage framebuffer(width, height, TGAImage::RGB);        // 创建帧缓冲

    // 遍历所有输入的模型文件
    for (int m=1; m<argc; m++) {
        Model model(argv[m]);                       // 加载模型数据
        PhongShader shader(light, model);
        // 遍历所有三角形面
        for (int f=0; f<model.nfaces(); f++) {
            Triangle clip = { shader.vertex(f, 0),  // 组装三角形图元
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // 光栅化图元到帧缓冲
        }
    }

    framebuffer.write_tga_file("framebuffer.tga"); // 保存渲染结果
    return 0;
}