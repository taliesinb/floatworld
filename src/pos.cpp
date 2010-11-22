#include <iostream>
#include "pos.hpp"

using namespace std;

ostream& operator<<(ostream& os, const Pos& p)
{
	os << p.row << " " << p.col;
}


istream& operator>>(istream& is, Pos& p)
{
	is >> p.row >> p.col;
}


ostream& operator<<(ostream& os, const FloatPos& p)
{
	os << p.row << " " << p.col;
}

istream& operator>>(istream& is, FloatPos& p)
{
	is >> p.row >> p.col;
}

