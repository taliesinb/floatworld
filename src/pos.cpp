#include <iostream>
#include "pos.hpp"
#include "metaclass.hpp"

using namespace std;

ostream& operator<<(ostream& os, const Pos& p)
{
  os << "[" << p.row << ", " << p.col << "]";
  return os;
}

istream& operator>>(istream& is, Pos& p)
{
  is >> "[" >> p.row >> ", " >> p.col >> "]";
  return is;
}

