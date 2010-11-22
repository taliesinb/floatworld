#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "matrix.hpp"
#include "metaclass.hpp"
#include "pos.hpp"

class Shape : public Class
{
  public:

    FloatPos pos;
    float threshold;
    float energy;
    float ratio;

    Shape();

    void RandomPosition();
    virtual int Area() = 0;

    void Inject(Matrix& m, int y, int x, float de)
    {
      float& e = m.GetW(y + pos.row, x + pos.col);
      if (e < threshold) e += de;
//      if (e > threshold) e = threshold;
//      elseif (e < -threshold) e = -threshold;
    }
    void Draw(Matrix& m);
    virtual void DrawFull(Matrix& m) = 0;
    virtual void DrawStochastic(Matrix& m, int n) = 0;
};

class Circle : public Shape
{
  public:

    float radius;

    Circle();
    
    int Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class GaussianCircle : public Circle
{
  public:

    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class Annalus : public Shape
{
  public:

    float radius1;
    float radius2;
    float angle1;
    float angle2;
    
    Annalus();
    
    int Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class Rectangle : public Shape
{
  public:

    int width;
    int length;
    float angle;

    Rectangle();

    int Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

#endif
