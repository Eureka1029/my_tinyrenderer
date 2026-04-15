#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 800;
constexpr int height = 800;

//求三角形面积
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

//画三角形
void triangle(int ax, int ay, double az, int bx, int by, double bz, int cx, int cy, double cz, double* zbuffer, TGAImage &framebuffer, TGAColor color) {
int bbminx = std::max(0, std::min({ax, bx, cx}));
    int bbminy = std::max(0, std::min({ay, by, cy}));
    int bbmaxx = std::min(width - 1, std::max({ax, bx, cx}));
    int bbmaxy = std::min(height - 1, std::max({ay, by, cy}));
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            double z = alpha * az + beta * bz + gamma * cz;
            int idx = x + y * width;
            if(zbuffer[idx] >= z) continue;
            zbuffer[idx] = z;
            framebuffer.set(x, y, color);
        }
    }
}

//透视投影
vec3 persp(vec3 v){
    constexpr int c = 3;
    return v / (1 - v.z/c);
}

//正交投影
std::tuple<int,int,double> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             v.z };
}

//旋转矩阵
vec3 rot(vec3 v) {
    constexpr double a = M_PI/6;
    const mat<3,3> Ry = {{{std::cos(a), 0, std::sin(a)}, {0,1,0}, {-std::sin(a), 0, std::cos(a)}}};
    return Ry*v;
}



int main(int argc, char** argv) {

    const char* filename = "obj/diablo3_pose/diablo3_pose.obj"; 

    if (argc == 2) {
        filename = argv[1];
    } else if (argc > 2) {
        // 防止乱传参数
        std::cerr << "Usage: " << argv[0] << " [obj/model.obj]" << std::endl;
        return 1;
    }

    Model model(filename);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer_image(width, height, TGAImage::GRAYSCALE);


    double* zbuffer = new double [width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<double>::max();
    }

    //遍历每个面,对每个点进行投影.
    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = project(persp(rot(model.vert(i, 0))));
        auto [bx, by, bz] = project(persp(rot(model.vert(i, 1))));
        auto [cx, cy, cz] = project(persp(rot(model.vert(i, 2))));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    //zbuffer数组画深度图
    double min_z = std::numeric_limits<double>::max();
    double max_z = -std::numeric_limits<double>::max();
    for(int i = 0; i < height * width; i++){
        if(zbuffer[i] != -std::numeric_limits<double>::max()){
            if(zbuffer[i] < min_z) min_z = zbuffer[i];
            if(zbuffer[i] > max_z) max_z = zbuffer[i];
        }
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int idx = x + y * width;
            double z = zbuffer[idx];
            if(z != -std::numeric_limits<double>::max()){
                unsigned char c = static_cast<unsigned char>((z - min_z) / (max_z - min_z) * 255);
                zbuffer_image.set(x, y, TGAColor({c, c, c, 255}));
            }
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer_image.write_tga_file("zbuffer.tga");
    delete[] zbuffer;
    return 0;
}