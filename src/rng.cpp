#include "rng.hpp"
#include "metaclass.hpp"

const int PRECISION = 2 << 16;
const int GAUSS_ITERS = 3;

RNG::RNG() : _w(1), _z(1)
{
}

void RNG::Seed(int seed)
{
    _w = seed * 5 + 1;
    _z = seed * 7 + 2;
    for (int k = 0; k < 10; k++) Integer(1);
}

int RNG::Integer(int n)
{
    _z = 36969 * (_z & 65535) + (_z >> 16);
    _w = 18000 * (_w & 65535) + (_w >> 16);
    return ((_z << 16) + _w) % (n + 1);
}

int RNG::Integer(int a, int b)
{
    return a + Integer(b - a);
}

float RNG::Float() {
    return float(Integer(PRECISION) % PRECISION) / PRECISION;
}

float RNG::Float(float max)
{
    return max * Float();
}

float RNG::Float(float a, float b)
{
    return a + Float(b - a);
}

float RNG::Gaussian()
{
    float x = 0;
    for (int i = 0; i < GAUSS_ITERS * GAUSS_ITERS; i++) x += Float();
    return 3.4739 * (x - (GAUSS_ITERS * GAUSS_ITERS) / 2.0) / GAUSS_ITERS;
}

float RNG::Gaussian(float mean, float sd)
{
    return mean + sd * Gaussian();
}

int RNG::Bit()
{
    return Integer(1);
}

Pos RNG::Position(int rows, int cols)
{
    return Pos(Integer(rows-1), Integer(cols-1));
}

int RNG::Dir()
{
    return Integer(3);
}

int RNG::Bit(float prob)
{
    return (Float() < prob) ? 1 : 0;
}

bool RNG::Bool()
{
    return Integer(1) == 0;
}

bool RNG::Bool(float prob)
{
    return (Float() < prob);
}

int RNG::Sign()
{
    return Bit() * 2 - 1;
}

std::ostream& operator<<(std::ostream& os, const RNG& rng)
{
    os << "[" << rng._w << ", " << rng._z << "]";
    return os;
}

std::istream& operator>>(std::istream& is, RNG& rng)
{
    is >> "[" >> rng._w >> ", " >> rng._z >> "]";
    return is;
}

