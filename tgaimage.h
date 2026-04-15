#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = {0,0,0,0};
    std::uint8_t bytespp = 4;
    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
    #include <algorithm> // 为了使用 std::min 和 std::max

    // 1. 重载乘号 *
    TGAColor operator*(const double value) const { // 加了 const，保证不会修改原始变量
        TGAColor result = *this; // 拷贝一份自己
        for(int i = 0; i < 4; i++){
            // 先计算浮点结果，再限制在 0-255 之间
            int val = result.bgra[i] * value;
            result.bgra[i] = std::max(0, std::min(255, val));
        }
        return result; // 返回新算出来的颜色
    }

    // 2. 重载加号 +
    TGAColor operator+(const TGAColor& other) const { // 加了 const
        TGAColor result = *this; // 拷贝一份自己
        for(int i = 0; i < 4; i++){
            // 防止相加后超过 255 导致颜色溢出变黑
            int sum = result.bgra[i] + other.bgra[i];
            result.bgra[i] = std::min(255, sum);
        }
        return result; // 返回新算出来的颜色
    }
};

struct TGAImage {
    enum Format { GRAYSCALE=1, RGB=3, RGBA=4 };
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {});
    bool  read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip=true, const bool rle=true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor &c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};

