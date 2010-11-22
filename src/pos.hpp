#ifndef POS_HPP
#define POS_HPP

#include "misc.hpp"
#include <math.h>
#include <iostream>

#define PI 3.14159265358979323

class FloatPos;

class Pos
{
  public:
    short row;
    short col;

    Pos() { }
    Pos(const FloatPos& pos);
    Pos(const Pos& pos) : row(pos.row), col(pos.col) { }
    Pos(short r, short c) : row(r), col(c) { }
    Pos(int dir)
    {
      switch(Mod(dir, 4)) {
        case 0: row = -1, col = 0; break;
        case 1: row = 0; col = 1;  break;
        case 2: row = 1; col = 0;  break;
        case 3: row = 0; col = -1; break;
      }
   }

    Pos Transpose() { return Pos(col, row); }

#define MAKE_OP(X)                                          \
    Pos operator X(int f) { return Pos(row X f, col X f); } \
    void operator X##=(int f) { row X##= f; col X##= f; } \
    Pos operator X(Pos f) { return Pos(row X f.row, col X f.col); } \
    void operator X##=(Pos f) { row X##= f.row; col X##= f.col; }

    MAKE_OP(+);
    MAKE_OP(-);
    MAKE_OP(*);
    MAKE_OP(/);

    float Mag() {
      return sqrt(row * row + col * col);
    }
    bool Inside(short rows, short cols) {
      return (row >= 0) && (row < rows) && (col >= 0) && (col < cols);
    }
    Pos Wrap(short rows, short cols) {
      return Pos(Mod(row, rows), Mod(col, cols));
    }
    bool operator==(Pos pos) {
      return row == pos.row && col == pos.col;
    }
};

class FloatPos
{
  public:
    float row;
    float col;

    FloatPos() { }
    FloatPos(const Pos& pos) : row(pos.row), col(pos.col) { }
    FloatPos(const FloatPos& pos) : row(pos.row), col(pos.col) { }
    FloatPos(float r, float c) : row(r), col(c) { }
    FloatPos(float a) : row(-sin(2 * PI * a)), col(cos(2 * PI * a)) { }

    FloatPos Transpose() { return FloatPos(col, row); }

    #undef MAKE_OP
    #define MAKE_OP(X) \
    FloatPos operator X(float f) { return FloatPos(row X f, col X f); } \
    void operator X##=(float f) { row X##= f; col X##= f; } \
    FloatPos operator X(FloatPos f) { return FloatPos(row X f.row, col X f.col); } \
    void operator X##=(FloatPos f) { row X##= f.row; col X##= f.col; }
    
    MAKE_OP(+);
    MAKE_OP(-);
    MAKE_OP(*);
    MAKE_OP(/);

    bool operator==(FloatPos pos) {
      return row == pos.row && col == pos.col;
    }
};

inline Pos::Pos(const FloatPos& pos) :
  row(round(pos.row)), col(round(pos.col))
{ }

std::ostream& operator<<(std::ostream& os, const Pos& p);
std::istream& operator>>(std::istream& is, Pos& p);

std::ostream& operator<<(std::ostream& os, const FloatPos& p);
std::istream& operator>>(std::istream& is, FloatPos& p);

#endif
