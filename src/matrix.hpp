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

    Matrix(std::istream& is);
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
  
    void MaskWith(const Matrix& m);
  
    const float& GetW(int r, int c) const { return data[Mod(r,rows) * cols + Mod(c,cols)]; }
    float& GetW(int r, int c) { return data[Mod(r,rows) * cols + Mod(c,cols)]; }
      
    float& Get(Pos p) { return data[p.row * cols + p.col]; }
    const float& Get(Pos p) const { return data[p.row * cols + p.col]; }
    float& Get(int r, int c=0) { return data[r * cols + c]; }
    const float& Get(int r, int c=0) const { return data[r * cols + c]; }
    float& operator()(Pos p) { return data[p.row * cols + p.col]; }
    const float& operator()(Pos p) const { return data[p.row * cols + p.col]; }
    float& operator()(int r, int c=0) { return data[r * cols + c]; }
    const float& operator()(int r, int c=0) const { return data[r * cols + c]; }
    const float* operator[](int row) const { return &data[row * cols]; }
    float* operator[](int row) { return &data[row * cols]; }

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
  
    struct PrettyWrapper
    {  const Matrix* m; };
    PrettyWrapper Pretty() const
    {
        PrettyWrapper w; w.m = this;
        return w;
    }
    void ToLatex(const char* file);
};

struct RLEpair
{
    int length;
    float value;
};

void EncodeRLE(RLEpair rle[], const Matrix& m);
void DecodeRLE(RLEpair rle[], Matrix& m);
void SwapContents(Matrix& m1, Matrix& m2);

void WriteMatrix(const char* file, const Matrix& m, bool append=false);
Matrix ReadMatrix(const char* file);
void WriteMatrices(const char* file, const std::vector<Matrix>& matrices);
void ReadMatrices(const char* file, std::vector<Matrix>& matrices);
void DrawMatrix(const char* file, const Matrix& m);
void DrawMatrices(const char* file, const std::vector<Matrix>& matrices, int overflow);

std::ostream& operator<<(std::ostream& os, const Matrix& m);
std::istream& operator>>(std::istream& is, Matrix& m);
std::ostream& operator<<(std::ostream& os, const Matrix::PrettyWrapper& m);

float SquareMetric(const Matrix& a, const Matrix& b);
float RootMetric(const Matrix& a, const Matrix& b);
float HammondMetric(const Matrix& a, const Matrix& b);

#endif
