#pragma once
// 几何库头文件 - 提供向量和矩阵的基本数学运算
// 包含 n 维向量模板类、向量操作符、矩阵模板类等实现
#include <cmath>
#include <cassert>
#include <iostream>

// n 维向量模板类 - 使用数组存储向量分量
template<int n> struct vec {
    double data[n] = {0};  // 向量分量数据
    // 索引操作符 - 获取向量分量
    double& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    // 索引操作符常量版本 - 读取向量分量
    double  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
};

// 向量点积 - 计算两个向量的数量积
// 注：使用反向循环是计算机图形早期的优化技巧，现在已不必要
template<int n> double operator*(const vec<n>& lhs, const vec<n>& rhs) {
    double ret = 0;                         
    for (int i=n; i--; ret+=lhs[i]*rhs[i]); 
    return ret;                             
}

// 向量加法 - 对应分量相加
template<int n> vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]+=rhs[i]);
    return ret;
}

// 向量减法 - 对应分量相减
template<int n> vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]-=rhs[i]);
    return ret;
}

// 向量与标量乘法 - 向量的所有分量与标量相乘
template<int n> vec<n> operator*(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]*=rhs);
    return ret;
}

// 标量与向量乘法 - 支持交换律 (标量*向量 = 向量*标量)
template<int n> vec<n> operator*(const double& lhs, const vec<n> &rhs) {
    return rhs * lhs;
}

// 向量除法 - 向量的所有分量均除以标量
template<int n> vec<n> operator/(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]/=rhs);
    return ret;
}

// 向量输出操作符 - 便于调试和打印
template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i=0; i<n; i++) out << v[i] << " ";
    return out;
}

// 2D向量特化 - 提供 x, y 命名成员变量
template<> struct vec<2> {
    double x = 0, y = 0;  // x、y分量
    // 用方括号访问分量，i=0为x，i=1为y
    double& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    double  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }
};

// 3D向量特化 - 提供 x, y, z 命名成员变量
template<> struct vec<3> {
    double x = 0, y = 0, z = 0;  // x、y、z分量
    // 用方括号访问分量，i=0为x，i=1为y，i=2为z
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

// 4D向量特化 - 提供齐次坐标支持 (x, y, z, w)
template<> struct vec<4> {
    double x = 0, y = 0, z = 0, w = 0;  // x、y、z、w分量
    // 用方括号访问分量
    double& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
    double  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
    // 提取前2个分量作为2D向量
    vec<2> xy()  const { return {x, y};    }
    // 提取前3个分量作为3D向量
    vec<3> xyz() const { return {x, y, z}; }
};

// 向量类型别名 - 方便使用
typedef vec<2> vec2;  // 2D向量
typedef vec<3> vec3;  // 3D向量
typedef vec<4> vec4;  // 4D向量(齐次坐标)

// 计算向量的模(长度) - 使用欧几里得距离公式
template<int n> double norm(const vec<n>& v) {
    return std::sqrt(v*v);
}

// 返回向量的单位向量 - 将向量归一化处理
template<int n> vec<n> normalized(const vec<n>& v) {
    return v / norm(v);
}

// 计算3D向量的叉积(外积) - 返回垂直于两个输入向量的向量
inline vec3 cross(const vec3 &v1, const vec3 &v2) {
    return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

// 前向声明行列式计算模板
template<int n> struct dt;

// n行m列矩阵模板类
template<int nrows,int ncols> struct mat {
    vec<ncols> rows[nrows] = {{}};  // 矩阵的行向量数组

    // 行索引操作符 - 访问矩阵的某一行
    vec<ncols>& operator[] (const int idx)       { assert(idx>=0 && idx<nrows); return rows[idx]; }
    // 行索引操作符常量版本
    const vec<ncols>& operator[] (const int idx) const { assert(idx>=0 && idx<nrows); return rows[idx]; }

    // 计算矩阵行列式
    double det() const {
        return dt<ncols>::det(*this);
    }

    // 计算矩阵的余子式
    double cofactor(const int row, const int col) const {
        mat<nrows-1,ncols-1> submatrix;  // 去掉指定行列后的子矩阵
        for (int i=nrows-1; i--; )
            for (int j=ncols-1;j--; submatrix[i][j]=rows[i+int(i>=row)][j+int(j>=col)]);
        return submatrix.det() * ((row+col)%2 ? -1 : 1);
    }

    // 计算矩阵的逆矩阵的转置
    mat<nrows,ncols> invert_transpose() const {
        mat<nrows,ncols> adjugate_transpose;  // 伴随矩阵的转置
        // 计算伴随矩阵中的每个余子式
        for (int i=nrows; i--; )
            for (int j=ncols; j--; adjugate_transpose[i][j]=cofactor(i,j));
        return adjugate_transpose/(adjugate_transpose[0]*rows[0]);
    }

    // 计算矩阵的逆矩阵
    mat<nrows,ncols> invert() const {
        return invert_transpose().transpose();
    }

    // 计算矩阵的转置矩阵
    mat<ncols,nrows> transpose() const {
        mat<ncols,nrows> ret;
        for (int i=ncols; i--; )
            for (int j=nrows; j--; ret[i][j]=rows[j][i]);
        return ret;
    }
};

// 行向量乘以矩阵
template<int nrows,int ncols> vec<ncols> operator*(const vec<nrows>& lhs, const mat<nrows,ncols>& rhs) {
    return (mat<1,nrows>{{lhs}}*rhs)[0];
}

// 矩阵乘以列向量
template<int nrows,int ncols> vec<nrows> operator*(const mat<nrows,ncols>& lhs, const vec<ncols>& rhs) {
    vec<nrows> ret;
    for (int i=nrows; i--; ret[i]=lhs[i]*rhs);
    return ret;
}

// 矩阵乘以矩阵
template<int R1,int C1,int C2>mat<R1,C2> operator*(const mat<R1,C1>& lhs, const mat<C1,C2>& rhs) {
    mat<R1,C2> result;
    // 使用三重循环计算矩阵乘积
    for (int i=R1; i--; )
        for (int j=C2; j--; )
            for (int k=C1; k--; result[i][j]+=lhs[i][k]*rhs[k][j]);
    return result;
}

// 矩阵乘以标量
template<int nrows,int ncols>mat<nrows,ncols> operator*(const mat<nrows,ncols>& lhs, const double& val) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; result[i] = lhs[i]*val);
    return result;
}

// 矩阵除以标量
template<int nrows,int ncols>mat<nrows,ncols> operator/(const mat<nrows,ncols>& lhs, const double& val) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; result[i] = lhs[i]/val);
    return result;
}

// 矩阵加法
template<int nrows,int ncols>mat<nrows,ncols> operator+(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; )
        for (int j=ncols; j--; result[i][j]=lhs[i][j]+rhs[i][j]);
    return result;
}

// 矩阵减法
template<int nrows,int ncols>mat<nrows,ncols> operator-(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; )
        for (int j=ncols; j--; result[i][j]=lhs[i][j]-rhs[i][j]);
    return result;
}

// 矩阵输出操作符 - 便于调试和打印
template<int nrows,int ncols> std::ostream& operator<<(std::ostream& out, const mat<nrows,ncols>& m) {
    for (int i=0; i<nrows; i++) out << m[i] << std::endl;
    return out;
}

// 行列式计算 - 使用模板元编程递归计算行列式
template<int n> struct dt {
    // 使用余子式展开法计算行列式
    static double det(const mat<n,n>& src) {
        double ret = 0;
        for (int i=n; i--; ret += src[0][i] * src.cofactor(0,i));
        return ret;
    }
};

// 1x1矩阵的行列式特化 - 递归终止条件
template<> struct dt<1> {
    // 对于1x1矩阵，行列式就是该单一元素
    static double det(const mat<1,1>& src) {
        return src[0][0];
    }
};

