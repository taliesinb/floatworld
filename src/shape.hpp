#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "world.hpp"

class Shape : public Occupant
{
  public:

    float threshold;
    float energy;
    float ratio;
    float p_jump;

    Shape();

    void Update();

    void Inject(Matrix& m, int y, int x, float de)
    {
      float& e = m.GetW(y + pos.row, x + pos.col);
      if (e < threshold) e += de;
      if (e > threshold) e = threshold;
      else if (e < -threshold) e = -threshold;
    }
    void Draw(Matrix& m);

    virtual float Area() = 0;
    virtual void DrawFull(Matrix& m) = 0;
    virtual void DrawStochastic(Matrix& m, int n) = 0;
};

class EnergyDisk : public Shape
{
  public:

    int radius;

    EnergyDisk();
    
    float Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class EnergyGaussian : public EnergyDisk
{
  public:

    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class EnergyAnnalus : public Shape
{
  public:

    float radius1;
    float radius2;
    
    EnergyAnnalus();
    
    float Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

class EnergyRectangle : public Shape
{
  public:

    int width;
    int length;
    float angle;

    EnergyRectangle();

    float Area();
    void DrawFull(Matrix& m);
    void DrawStochastic(Matrix& m, int n);
};

#endif
