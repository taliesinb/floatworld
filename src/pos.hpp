#ifndef POS_HPP
#define POS_HPP

#include "misc.hpp"
#include <math.h>
#include <iostream>

#define PI 3.14159265358979323

class Pos
{
  public:
    short row;
    short col;

    Pos() { }
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
std::ostream& operator<<(std::ostream& os, const Pos& p);
std::istream& operator>>(std::istream& is, Pos& p);

#endif
