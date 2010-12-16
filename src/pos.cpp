#include <iostream>
#include "pos.hpp"
#include "metaclass.hpp"

using namespace std;

ostream& operator<<(ostream& os, const Pos& p)
{
    if (human_readable)
        os << "[" << p.row << ", " << p.col << "]";
    else
        os << p.row << " " << p.col << " ";
    return os;
}

istream& operator>>(istream& is, Pos& p)
{
    if (human_readable)
        is >> "[" >> p.row >> ", " >> p.col >> "]";
    else
        is >> p.row >> p.col;
    return is;
}

