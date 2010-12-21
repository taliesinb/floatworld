#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "pos.hpp"
#include <math.h>
#include <iostream>
#include <vector>


class Matrix {
public:
    short cols, rows;
    float* data;

    Matrix();
    Matrix(int rws, int cls, float* dat=NULL);
    Matrix(const Matrix& m);

    void Resize(int rws, int cls);

    ~Matrix();

    int Len() const;
  
    void Set(const float* x);
    void SetRandom(float min, float max);
    void SetBetween(int r, int c, int dr, int dc, int n, float constant = 0);
    void SetSubMatrix(int r1, int c1, int r2, int c2, float value);
    void SetConstant(float con);
    void SetZero();
    void SetChess();

    void Transpose();
    void ReverseRows();
    void ReverseCols();
    void Rotate(int d);

    float GetL2Norm() const;
    float GetHammingNorm() const;
    float GetMag() const;
    float GetMax() const;
    float GetMin() const;
    float GetTotal() const;
    Matrix GetSubMatrix(Pos start, Pos size) const;

    void Apply(float f(float));
    void Apply(double f(double));
  
    inline const float& GetW(int r, int c) const { return data[Mod(r,rows) * cols + Mod(c,cols)]; }
    inline float& GetW(int r, int c) { return data[Mod(r,rows) * cols + Mod(c,cols)]; }
      
    inline float& operator()(Pos p) { return data[p.row * cols + p.col]; }
    inline const float& operator()(Pos p) const { return data[p.row * cols + p.col]; }
    inline float& operator()(int r, int c=0) { return data[r * cols + c]; }
    inline const float& operator()(int r, int c=0) const { return data[r * cols + c]; }
    inline const float* operator[](int row) const { return &data[row * cols]; }
    inline float* operator[](int row) { return &data[row * cols]; }

    void operator=(const Matrix& m);
    void operator=(float c);

    bool operator==(const Matrix& m) const;
  
    Matrix operator*(const Matrix& m) const;
    void operator*=(const Matrix& m);
    Matrix operator*(float c) const;
    void operator*=(float c);
  
    Matrix operator+(const Matrix& m) const;
    void operator+=(const Matrix& m);
    Matrix operator+(float c) const;
    void operator+=(float c);
  
    Matrix operator-(const Matrix& m) const;
    void operator-=(const Matrix& m);
    Matrix operator-(float c) const;
    void operator-=(float c);
};

void SwapContents(Matrix& m1, Matrix& m2);

std::ostream& operator<<(std::ostream& os, const Matrix& m);
std::istream& operator>>(std::istream& is, Matrix& m);

#endif
