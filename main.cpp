#include "our_gl.h"
#include "model.h"

// OpenGL 状态矩阵：视图、模型、透视投影
extern mat<4,4> Viewport, ModelView, Perspective;
// 深度缓冲区
extern std::vector<double> zbuffer;

// 空白着色器：只返回白色，不进行任何光照计算
struct BlankShader : IShader {
    const Model &model;

    BlankShader(const Model &m) : model(m) {}

    // 顶点着色器：计算顶点在裁剪空间的位置
    virtual vec4 vertex(const int face, const int vert) {
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;
    }

    // 片段着色器：返回白色
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, {255, 255, 255, 255}};
    }
};

// Phong 着色器：实现 Phong 光照模型
struct PhongShader : IShader {
    const Model &model;
    // 眼睛空间中的光线方向
    vec4 l;
    // 三角形三个顶点的纹理坐标
    vec2  varying_uv[3];
    // 三角形三个顶点的法线在眼睛空间中的坐标
    vec4 varying_nrm[3];
    // 三角形三个顶点在视角坐标中的位置
    vec4 tri[3];

    PhongShader(const vec3 light, const Model &m) : model(m) {
        // 将光线向量转换到眼睛坐标系
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    // 顶点着色器：计算每个顶点的属性并返回裁剪空间坐标
    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert]  = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position;
        // 返回裁剪坐标
        return Perspective * gl_Position;
    }

    // 片段着色器：使用 Phong 光照模型计算最终像素颜色
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        // 计算切线空间的变换矩阵
        mat<2,4> E = { tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U = { varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        // 构建 Darboux 坐标系
        mat<4,4> D = {normalized(T[0]),  // 切线向量
                      normalized(T[1]),  // 副法线向量
                      // 插值法线
                      normalized(varying_nrm[0]*bar[0] + varying_nrm[1]*bar[1] + varying_nrm[2]*bar[2]),
                      {0,0,0,1}}; // Darboux 坐标系基向量
        // 计算像素的纹理坐标
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        // 从法线贴图获取法线
        vec4 n = normalized(D.transpose() * model.normal(uv));
        // 计算反射光线方向
        vec4 r = normalized(n * (n * l)*2 - l);
        // 环境光强度
        double ambient  = .4;
        // 漫反射光强度
        double diffuse  = 1.*std::max(0., n * l);
        // 镜面光强度
        // 注意：相机在 z 轴上（眼睛坐标系），所以 (0,0,1)*(r.x, r.y, r.z) = r.z
        double specular = (1.+3.*sample2D(model.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);
        // 获取基础颜色
        TGAColor gl_FragColor = sample2D(model.diffuse(), uv);
//      TGAColor gl_FragColor = {255, 255, 255, 255};
        // 应用光照计算
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        // 不丢弃该像素
        return {false, gl_FragColor};
    }
};

// 将深度缓冲区数据保存为灰度图
void drop_zbuffer(std::string filename, std::vector<double> &zbuffer, int width, int height) {
    TGAImage zimg(width, height, TGAImage::GRAYSCALE, {0,0,0,0});
    // 找到最小和最大深度值用于归一化
    double minz = +1000;
    double maxz = -1000;
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            double z = zbuffer[x+y*width];
            if (z<-100) continue;  // 跳过背景像素
            minz = std::min(z, minz);
            maxz = std::max(z, maxz);
        }
    }
    // 将深度值映射到灰度值
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            double z = zbuffer[x+y*width];
            if (z<-100) continue;
            z = (z - minz)/(maxz-minz) * 255;
            zimg.set(x, y, {(unsigned char)z, 255, 255, 255});
        }
    }
    zimg.write_tga_file(filename);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    // 输出图像尺寸
    constexpr int width  = 800;
    constexpr int height = 800;
    // 阴影贴图缓冲区尺寸
    constexpr int shadoww = 8000;
    constexpr int shadowh = 8000;
    // 光源位置
    constexpr vec3  light{ 1, 1, 1};
    // 相机位置
    constexpr vec3    eye{-1, 0, 2};
    // 相机看向位置
    constexpr vec3 center{ 0, 0, 0};
    // 相机上向量
    constexpr vec3     up{ 0, 1, 0};

    // 常规渲染通道
    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    // 创建帧缓冲区并用背景颜色初始化
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
//    TGAImage framebuffer(width, height, TGAImage::RGB);

    // 对所有输入对象进行迭代
    for (int m=1; m<argc; m++) {
        // 加载模型数据
        Model model(argv[m]);
        PhongShader shader(light, model);
        // 对所有面进行迭代
        for (int f=0; f<model.nfaces(); f++) {
            // 组装图元
            Triangle clip = { shader.vertex(f, 0),
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            // 光栅化图元
            rasterize(clip, shader, framebuffer);
        }
    }
    framebuffer.write_tga_file("framebuffer.tga");
    drop_zbuffer("zbuffer1.tga", zbuffer, width, height);

    // 创建阴影遮罩
    std::vector<bool> mask(width*height, false);
    std::vector<double> zbuffer_copy = zbuffer;
    mat<4,4> M = (Viewport * Perspective * ModelView).invert();

    { // 阴影渲染通道
        lookat(light, center, up);
        init_perspective(norm(eye-center));
        init_viewport(shadoww/16, shadowh/16, shadoww*7/8, shadowh*7/8);
        init_zbuffer(shadoww, shadowh);
        TGAImage trash(shadoww, shadowh, TGAImage::RGB, {177, 195, 209, 255});

        // 对所有输入对象进行迭代
        for (int m=1; m<argc; m++) {
            // 加载模型数据
            Model model(argv[m]);
            BlankShader shader{model};
            // 对所有面进行迭代
            for (int f=0; f<model.nfaces(); f++) {
                // 组装图元
                Triangle clip = { shader.vertex(f, 0),
                                  shader.vertex(f, 1),
                                  shader.vertex(f, 2) };
                // 光栅化图元
                rasterize(clip, shader, trash);
            }
        }
        trash.write_tga_file("shadowmap.tga");
    }

    drop_zbuffer("zbuffer2.tga", zbuffer, shadoww, shadowh);

    mat<4,4> N = Viewport * Perspective * ModelView;


    // 后处理：计算阴影遮罩
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            // 从屏幕坐标转换回世界坐标
            vec4 fragment = M * vec4{(double)x, (double)y, zbuffer_copy[x+y*width], 1.};
            // 投影到阴影贴图空间
            vec4 q = N * fragment;
            vec3 p = q.xyz()/q.w;
            // 检查像素是否被光线照亮
            bool lit =  (fragment.z<-100 ||                                   // 是背景或
                        (p.x<0 || p.x>=shadoww || p.y<0 || p.y>=shadowh) ||   // 超出阴影缓冲区范围
                        (p.z > zbuffer[int(p.x) + int(p.y)*shadoww] - .03));  // 在阴影贴图中可见
            mask[x+y*width] = lit;
        } 
    }

    // 保存遮罩图
    TGAImage maskimg(width, height, TGAImage::GRAYSCALE);
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            if (mask[x+y*width]) continue;
            maskimg.set(x, y, {255, 255, 255, 255});
        }
    }
    maskimg.write_tga_file("mask.tga");

    // 对阴影区域进行暗化处理
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            if (mask[x+y*width]) continue;
            TGAColor c = framebuffer.get(x, y);
            vec3 a = {(double)c[0], (double)c[1], (double)c[2]};
            if (norm(a)<80) continue;
            a = normalized(a)*80;
            framebuffer.set(x, y, { (unsigned char)a[0], (unsigned char)a[1], (unsigned char)a[2], 255 });
        }
    }
    framebuffer.write_tga_file("shadow.tga");


    return 0;
}