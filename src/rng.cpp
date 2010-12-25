#include "rng.hpp"

const int PRECISION = 2 << 16;
const int GAUSS_ITERS = 3;

RNG::RNG() : _w(1), _z(1)
{
}

int RNG::Integer(int n)
{
    _z = 36969 * (_z & 65535) + (_z >> 16);
    _w = 18000 * (_w & 65535) + (_w >> 16);
    return ((_z << 16) + _w) % n;
}

int RNG::Integer(int a, int b)
{
    return a + Integer(b - a);
}

float RNG::Float() {
    return float(Integer() % PRECISION) / PRECISION;
}

float RNG::Float(float max)
{
    return max * Float();
}

float RNG::Float(float a, float b)
{
    return a + Float(b - a);
}

bool RNG::Bool(float p)
{
    return Float() < p;
}

float RNG::Gaussian()
{
    float x = 0;
    for (int i = 0; i < GAUSS_ITERS * GAUSS_ITERS; i++) x += RandFloat();
    return 3.4739 * (x - (GAUSS_ITERS * GAUSS_ITERS) / 2.0) / GAUSS_ITERS;
}

float RNG::Gaussian(float mean, float sd)
{
    return mean + sd * Gaussian();
}

int RNG::Bit()
{
    return RandInt(1);
}

int RNG::Bit(float prob)
{
    return (Float() < prob) ? 1 : 0;
}

bool RNG::Bool()
{
    return RandInt(1) == 0;
}

bool RNG::Bool(float prob)
{
    return (Float() < prob);
}


