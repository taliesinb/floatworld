#include "matrix.hpp"
#include "misc.hpp"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "metaclass.hpp"

using namespace std;

Matrix::Matrix() : cols(0), rows(0), data(NULL)
{
}

Matrix::Matrix(int rws, int cls, float* dat)
{
    cols = cls;
    rows = rws;
    data = new float[cols * rows];
    if (dat) Set(dat);
}

Matrix::Matrix(const Matrix& m)
{
    cols = m.cols;
    rows = m.rows;
    data = new float[cols * rows];
    Set(m.data);
}

void Matrix::Resize(int rws, int cls)
{
    if (rws == rows && cols == cls) return;

    if (data) delete[] data;
    rows = rws;
    cols = cls;
    data = new float[cols * rows];
}

Matrix::~Matrix()
{
    if (data) delete[] data;
    data = NULL; // just to be safe
}

int Matrix::Len() const
{
    return cols * rows;
}

void Matrix::Set(const float* x)
{
    memcpy(data, x, cols * rows * sizeof(float));
}

void Matrix::SetRandom(float min, float max)
{
    for (int i = 0; i < cols * rows; i++) data[i] = RandFloat(min, max);
}

void Matrix::SetBetween(int r, int c, int dr, int dc, int n, float value)
{
    if (n == -1) n = 1e8;
    for (int i = 0; i < n && r < rows && c < cols; i++, r += dr, c += dc) {
        (*this)(r,c) = value;
    }
}

void Matrix::SetSubMatrix(int r1, int c1, int r2, int c2, float value)
{
    for (int r = r1; r <= r2; r++)
        for (int c = c1; c <= c2; c++)
            (*this)(r,c) = value;
}

void Matrix::SetConstant(float con)
{
    for(int i = 0; i < cols * rows; i++) data[i] = con;
}

void Matrix::SetZero()
{
    for(int i = 0; i < cols * rows; i++) data[i] = 0;
}

void Matrix::SetChess()
{
    for(int i = 0, x = 1; i < cols * rows; i++, x ^= 1) data[i] = x;
}

void  Matrix::Transpose()
{
    float* temp = new float[rows * cols];
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            temp[j * rows + i] = data[i * cols + j];
    delete [] data;
    data = temp;
    short t = rows;
    rows = cols;
    cols = t;
}

void Matrix::ReverseRows()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols / 2; j++)
        {
            float t = (*this)(i,j);
            (*this)(i,j) = (*this)(i,cols - 1 - j);
            (*this)(i, cols - 1 - j) = t;
        }
}

void Matrix::ReverseCols()
{
    for (int j = 0; j < cols; j++)
        for (int i = 0; i < rows / 2; i++)
        {
            float t = (*this)(i,j);
            (*this)(i,j) = (*this)(rows - 1 - i, j);
            (*this)(rows - 1 - i,j) = t;
        }
}

void Matrix::Rotate(int d)
{
    switch (Mod(d,4))
    {
        case 0:
            break;
        case 1:
            Transpose();
            ReverseRows();
            break;
        case 2:
            ReverseRows();
            ReverseCols();
            break;
        case 3:
            Transpose();
            ReverseCols();
            break;
    }
}

float Matrix::GetMag() const
{
    float mag = 0.0;
    for(int i = 0; i < cols * rows; i++)
        if (fabs(data[i]) > mag) mag = fabs(data[i]);
    return mag;
}  

float Matrix::GetMax() const
{
    float max = -1e12;
    for(int i = 0; i < cols * rows; i++)
        if (data[i] > max) max = data[i];
    return max;
}  

float Matrix::GetMin() const
{
    float min = 1e12;
    for(int i = 0; i < cols * rows; i++)
        if (data[i] < min) min = data[i];
    return min;
}

float Matrix::GetTotal() const
{
    float total = 0;
    for (int i = 0; i < cols * rows; i++)
        total += data[i];
    return total;
}

Matrix  Matrix::GetSubMatrix(Pos start, Pos size) const
{
    Matrix m(size.row, size.col);
    for (int i = 0; i < size.row; i++)
        for (int j = 0; j < size.col; j++)
            m(i,j) = (*this)(start.row  + i, start.col + j);
    return m;
}   

void Matrix::Apply(float f(float))
{
    for (int i = 0; i < cols * rows; i++)
        data[i] = f(data[i]);
}

void Matrix::Apply(double f(double))
{
    for (int i = 0; i < cols * rows; i++)
        data[i] = f(data[i]);
}

void Matrix::operator=(const Matrix& m)
{
    if (m.cols * m.rows != cols * rows) Resize(m.rows, m.cols);
   
    Set(m.data);
}

void Matrix::operator=(float c)
{
    SetConstant(c);
}

bool Matrix::operator==(const Matrix& m) const
{
    if (rows != m.rows || cols != m.cols) return false;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if ((*this)(i,j) != m(i,j)) return false;
    return true;
}

Matrix Matrix::operator*(const Matrix& m) const
{
    assert (cols == m.rows);
    Matrix result(rows, m.cols);
    for  (int i = 0; i < m.cols; i++)
        for  (int j = 0; j < rows; j++) {
            float& v = result(i,j);
            v = 0;
            for (int k = 0; k < cols; k++) v += (*this)(j,k) * m(k,i);
        }
    return result;
}

void Matrix::operator*=(const Matrix& m)
{
    *this = *this * m;
}

Matrix Matrix::operator*(float c) const {
    Matrix result(rows, cols);
    for (int i = 0; i < cols * rows; i++)
        result.data[i] = data[i] * c;
  
    return result;
}

void Matrix::operator*=(float c) {
    for (int i = 0; i < cols * rows; i++)
        data[i] *= c;
}

Matrix Matrix::operator+(const Matrix& m) const {
    assert (cols == m.cols && rows == m.rows);

    Matrix result(rows, cols);
    for (int i = 0; i < cols * rows; i++)
        result.data[i] = data[i] + m.data[i];
  
    return result;
}

void Matrix::operator+=(const Matrix& m) {
    assert (cols == m.cols && rows == m.rows);

    for (int i = 0; i < cols * rows; i++)
        data[i] += m.data[i];
}

Matrix Matrix::operator+(float c) const {
    Matrix result(rows, cols);
    for (int i = 0; i < cols * rows; i++)
        result.data[i] = data[i] + c;
  
    return result;
}

void Matrix::operator+=(float c) {
    for (int i = 0; i < cols * rows; i++)
        data[i] += c;
}


Matrix Matrix::operator-(const Matrix& m) const {
    assert (cols == m.cols && rows == m.rows);

    Matrix result(rows, cols);
    for (int i = 0; i < cols * rows; i++)
        result.data[i] = data[i] - m.data[i];
  
    return result;
}

void Matrix::operator-=(const Matrix& m) {
    assert (cols == m.cols && rows == m.rows);

    for (int i = 0; i < cols * rows; i++)
        data[i] -= m.data[i];
}

Matrix Matrix::operator-(float c) const {
    Matrix result(rows, cols);
    for (int i = 0; i < cols * rows; i++)
        result.data[i] = data[i] - c;
  
    return result;
}

void Matrix::operator-=(float c) {
    for (int i = 0; i < cols * rows; i++)
        data[i] -= c;
}

void SwapContents(Matrix& m1, Matrix& m2)
{
    float* temp = m1.data;
    m1.data = m2.data;
    m2.data = temp;
}

ostream& operator<<(ostream& os, const Matrix& m)
{
    os << "[";
    for (int i = 0; i < m.rows; i++)
    {
        if (i != 0) os << "], ";
        os << endl << "\t[";
        for (int j = 0; j < m.cols; j++)
        {
            if (j != 0) os << ", ";
            os << m.data[i * m.cols + j];
        }
    }
    os << "]\n]";
    return os;
}

istream& operator>>(istream& is, Matrix& m)
{
    list<list<float> > entries;
    is >> entries;

    int cols = entries.front().size();
    int rows = entries.size();
    for_iterate(r, entries)
        assert (cols == r->size());

    m.Resize(rows, cols);
    int i = 0;
    for_iterate(row, entries)
    {
        list<float>& r = *row;
        for_iterate(col, r)
        {
            m.data[i++] = *col;
        }
    }

    return is;
}

float Matrix::GetL2Norm() const
{
    double d = 0;
    for (int i = 0; i < rows * cols; i++)
        d += data[i] * data[i];
    return sqrt(d);
}

float Matrix::GetHammingNorm() const
{
    float d = 0;
    for (int i = 0; i < rows * cols; i++)
        if (data[i]) d++;
    return d;
}
