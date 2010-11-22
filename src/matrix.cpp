#include "matrix.hpp"
#include "misc.hpp"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

Matrix::Matrix(istream& is)
{
    is >> rows >> cols;
    data = new float[cols * rows];
    for (int i = 0; i < rows * cols; i++)
        is >> data[i];
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
    if (rws == rows  && cols == cls) return;
   
    delete[] data;
    rows = rws;
    cols = cls;
    data = new float[cols * rows];
}

Matrix::~Matrix()
{
    delete[] data;
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
        Get(r,c) = value;
    }
}

void Matrix::SetSubMatrix(int r1, int c1, int r2, int c2, float value)
{
    for (int r = r1; r <= r2; r++)
        for (int c = c1; c <= c2; c++)
            Get(r,c) = value;
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
            float t = Get(i,j);
            Get(i,j) = Get(i,cols - 1 - j);
            Get(i, cols - 1 - j) = t;
        }
}

void Matrix::ReverseCols()
{
    for (int j = 0; j < cols; j++)
        for (int i = 0; i < rows / 2; i++)
        {
            float t = Get(i,j);
            Get(i,j) = Get(rows - 1 - i, j);
            Get(rows - 1 - i,j) = t;
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
            m.Get(i,j) = Get(start.row  + i, start.col + j);
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

void Matrix::MaskWith(const Matrix& m)
{
    assert(m.cols == cols && m.rows == rows);

    for (int i = 0; i < cols * rows; i++)
        data[i] *= m.data[i];
}

// float& Matrix::GetRotated(int r, int c, int dir)
// {
//    switch(Mod(dir, 4)) {
//       case 0: return Get(r, c); 
//       case 1:  return Get((cols - 1) - c, r); 
//       case 2: return Get((rows - 1) - r, (cols - 1) - c);
//       case 3:  return Get(c, (rows - 1) - r);
//    }
// }

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
            if (Get(i,j) != m.Get(i,j)) return false;
    return true;
}

Matrix Matrix::operator*(const Matrix& m) const
{
    assert (cols == m.rows);
    Matrix result(rows, m.cols);
    for  (int i = 0; i < m.cols; i++)
        for  (int j = 0; j < rows; j++) {
            float& v = result.Get(i,j);
            v = 0;
            for (int k = 0; k < cols; k++) v += Get(j,k) * m.Get(k,i);
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

void Matrix::ToLatex(const char* file)
{
    ofstream os;
    os.open(file);
   
    os.precision(2);
    os.width(6);
    os.setf(ios::right | ios::fixed);

    os << "\\begin{pmatrix}" << endl;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            os << " " << Get(i,j) << " ";
            if (j != cols-1) os << "&";
        }
        if (i != rows-1) os << "\\\\" << endl;
    }
    os << endl << "\\end{pmatrix}" << endl;
    os.close();
}     


void SwapContents(Matrix& m1, Matrix& m2)
{
    float* temp = m1.data;
    m1.data = m2.data;
    m2.data = temp;
}

ostream& operator<<(ostream& os, const Matrix::PrettyWrapper& w)
{
    const Matrix& m = *w.m;
    os << "\n(";
    for(int i = 0; i < m.rows; i++) {
        for(int j = 0; j < m.cols; j++) {
            os.precision(2);
            os.width(6);
            os.setf(ios::right | ios::fixed);
            os << m.Get(i,j);
            if (j != m.cols - 1) os << ",";
        }
        os << ";";
        if (i != m.rows - 1) os << "\n ";
    }
    os << " )";
    return os;
}

ostream& operator<<(ostream& os, const Matrix& m)
{
    os << m.rows << " " << m.cols << endl;
    os.precision(50);
    for (int i = 0; i < m.rows * m.cols; i++)
        os << m.data[i] << " ";
    return os;
}

istream& operator>>(istream& is, Matrix& m)
{
    int rs, cs;
    is >> rs >> cs;
    assert(rs == m.rows && cs == m.cols);
    for (int i = 0; i < rs * cs; i++)
        is >> m.data[i];
    return is;
}


void EncodeRLE(RLEpair rle[], const Matrix& m)
{
    int sz = m.Len();
    int cell = 0;
    int index = 0;
    int count = 1;
    float old, cur = m.data[0];
    float* data = m.data;
    while (cell++ < sz)
    {
        old = cur;
        cur = data[cell] - (cell == sz) * 1e9;
        if (cur != old)
        {
            rle[index].length = count;
            rle[index].value = old;
            index++;
            count = 1;
        } else count++;
    }
}       

void DecodeRLE(RLEpair rle[], Matrix& m)
{
    int pos = 0, i = 0;
    while (pos < m.Len())
    {
        float v = rle[i].value;
        int l = rle[i].length;
        i++;
        while (l--)
        {
            m.data[pos] = v;
            pos++;
        }
    }
}


#define COUT(X) cout << X << endl               
\
void MatrixTest() {
    cout << "--- TESTING VECTORS --- \n\n";
 
    Matrix a(3,1), b(3,1);

    float d[3] = {0.0, 1.0, 0.0};
    b.Set(d);
    a.SetRandom(-1.0, 1.0); 

    PRINT(a);
    PRINT(b);
    PRINT(a + b);
    PRINT(a.Len());
    cout << "\n";
    PRINTDO(a.Apply(atan));
    PRINT(a);
    PRINTDO(a = b);
    PRINT(a);
  
    cout << "\n--- TESTING MATRICES --- \n\n";

    Matrix m(3,3), n(3,3);
    m.SetChess();
    n.SetRandom(-1.0, 1.0);

    PRINT(m);
    PRINT(n);

    PRINTDO(m = n);

    PRINT(m);
    PRINT(n * a);

    cout << "------------------\n";
}

void WriteMatrix(const char* file, const Matrix& m, bool append)
{
    ofstream os;
    if (append) os.open(file, ios_base::app);
    else os.open(file);
    os << true << endl;
    os << m << endl;
    flush(os);
}

Matrix ReadMatrix(const char* file)
{
    ifstream is;
    bool valid;
   
    is.open(file);
    is >> valid;
    assert(valid);
    return Matrix(is);
}

void WriteMatrices(const char* file, const vector<Matrix>& matrices)
{
    int len = matrices.size();
    ofstream os;
   
    os.open(file);

    for (int i = 0; i < len; i++)
    {
        os << true << endl;
        os << matrices[i] << endl;
    }
    os << false << endl;
    flush(os);
}

void ReadMatrices(const char* file, vector<Matrix>& matrices)
{
    ifstream is;

    is.open(file);

    while (1)
    {
        bool valid;
        is >> valid;
        if (!valid || is.fail()) break;
        matrices.push_back(Matrix(is));
    }
}

float HammondMetric(const Matrix& a, const Matrix& b)
{
    assert (a.rows == b.rows && a.cols == b.cols);
    float f = 0;
    float* data1 = a.data;
    float* data2 = b.data;
    int sz = a.rows * a.cols;
    for  (int i = 0; i < sz; i++)
    {
        if (data1[i] != data2[i]) f++;
    }
    return f;
}

float SquareMetric(const Matrix& a, const Matrix& b)
{
    assert (a.rows == b.rows && a.cols == b.cols);
    float f = 0;
    float* data1 = a.data;
    float* data2 = b.data;
    for  (int i = 0; i < a.rows * a.cols; i++)
    {
        float d = data1[i] - data2[i];
        f += d * d;
    }
    return sqrt(f);
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

float RootMetric(const Matrix& a, const Matrix& b)
{
    assert (a.rows == b.rows && a.cols == b.cols);
    float f = 0;
    float* data1 = a.data;
    float* data2 = b.data;
    for  (int i = 0; i < a.rows * a.cols; i++)
    {
        float d = data1[i] - data2[i];
        f += sqrt(fabs(d));
    }
    return f;
}
