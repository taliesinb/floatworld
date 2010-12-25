#ifndef RNG_HPP
#define RNG_HPP

// Simple multiply-with-carry RNG, basically straight from wikipedia

class RNG
{
private:
    unsigned int _w, _z;

public:
    int Integer(int n);
    int Integer(int a, int b);

    float Float();
    float Float(float max);
    float Float(float min, float max);

    float Gaussian();
    float Gaussian(float mean, float sd);

    bool Bool();
    bool Bool(float prob);

    int Bit();
    int Bit(float prob);

    int Sign();

    RNG();
};


#endif // RNG_HPP
