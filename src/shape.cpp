#include "shape.hpp"
#include "matrix.hpp"
#include "grid.hpp"

RegisterAbstractClass(Shape, Occupant);
RegisterVar(Shape, threshold);
RegisterVar(Shape, energy);
RegisterVar(Shape, ratio);
RegisterVar(Shape, p_jump);

RegisterQtHook(Shape, threshold, "Emax", FloatHook(-50,50,0.25));
RegisterQtHook(Shape, energy, "Einc", FloatHook(-5,5,0.1));
RegisterQtHook(Shape, ratio,  "fill", FloatHook(0,1,0.1));
RegisterQtHook(Shape, p_jump, "pjump", FloatHook(0, 1, 0.001));

RegisterClass(Circle, Shape);
RegisterVar(Circle, radius);
RegisterQtHook(Circle, radius, "radius", IntegerHook(0, 100));

RegisterClass(GaussianCircle, Circle);

/*
RegisterClass(GaussianCircle, Circle);

RegisterClass(Annalus, Shape);
RegisterVar(Annalus, radius1);
RegisterVar(Annalus, radius2);
RegisterVar(Annalus, angle1);
RegisterVar(Annalus, angle2);

RegisterClass(Rectangle, Shape);
RegisterVar(Rectangle, width);
RegisterVar(Rectangle, length);
RegisterVar(Rectangle, angle);
*/

Shape::Shape()
  : threshold(15),
    energy(1.0),
    ratio(1.0),
    p_jump(0.01)
{
    solid = false;
}

void Shape::Draw(Matrix& m)
{
  if (ratio < 1.0)
    DrawStochastic(m, ratio * Area());
  else
    DrawFull(m);
}

void Shape::Update()
{
    Draw(grid->energy);
    if (RandBool(p_jump))
        MoveRandom();
}

Circle::Circle()
{
    radius = 10;
}

float Circle::Area()
{
    return 3.141519 * radius * radius;
}

void Circle::DrawFull(Matrix& m)
{
    float r2 = radius * radius;
    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
            if (abs(i) + abs(j) < radius || (i * i) + (j * j) < r2)
                Inject(m, i, j, energy);
}
   

void Circle::DrawStochastic(Matrix& m, int n)
{
    float r2 = radius * radius;
    float de = energy / ratio;
    for (int t = 0; t < n; t++)
    {
        int r, c;
        do {
            r = RandInt(-radius, radius);
            c = RandInt(-radius, radius);
        } while (r * r + c * c > r2);
        Inject(m, r, c, de);
    }
}

/*
Annalus::Annalus()
  : radius1(5),
    radius2(10),
    angle1(0),
    angle2(0.5)
{
  
}

int Annalus::Area()
{
  return 3.14159 * (radius2 * radius2 - radius1 * radius1);
}

void Annalus::DrawFull(Matrix& m)
{
  float ir2 = radius1 * radius1;
  float or2 = radius2 * radius2;
  angle1 = FMod(angle1 + 0.003, 1);
  angle2 = FMod(angle2 + 0.003, 1);
  float a1 = angle1 > 0.5 ? (angle1 - 1) : angle1;
  float a2 = angle2 > 0.5 ? (angle2 - 1) : angle2;
  if (a1 < a2)
  {
    for (int i = -radius2; i <= radius2; i++)
      for (int j = -radius2; j <= radius2; j++)
      {
        float d = i * i + j * j;
        float a = atan2(j,-i) / (2 * 3.14159265358);
        if ((a >= a1 && a <= a2) &&
        (d >= ir2 && d < or2))
          Inject(m, i, j, energy);
      }
  } else
  {
    for (int i = -radius2; i <= radius2; i++)
      for (int j = -radius2; j <= radius2; j++)
      {
        float d = i * i + j * j;
        float a = atan2(j,-i) / (2 * 3.14159265358);
        if ((a <= a2 || a >= a1) &&
        (d >= ir2 && d < or2))
          Inject(m, i, j, energy);
      }
  }
}

void Annalus::DrawStochastic(Matrix& m, int n)
{
  float ir2 = radius1 * radius1;
  float or2 = radius2 * radius2;
  float de = energy / ratio;
  for (int t = 0; t < n; t++)
  {
    int r, c, d;
    float a;
    do {
      r = RandSign() * RandInt(radius1, radius2);
      c = RandSign() * RandInt(radius1, radius2);
      d = r * r + c * c;
      a = atan2(c,-r);
    } while (a < angle1 || a > angle2 || d < ir2 || d > or2);
    Inject(m, r, c, de);
  }
}

Rectangle::Rectangle()
  : length(20), width(3), angle(RandFloat())
{  
}

void Rectangle::DrawFull(Matrix& m)
{
  float ang = FMod(2 * angle, 1) / 2;
  bool xy = false;
  int hf = 1;
  if (ang > 0.375)
  {
    ang = 0.5 - ang;
    hf = -1;
  }
  if (ang > 0.125)
  {
    ang = 0.25 - ang;
    xy = true;
  }
  float tx = length * sin(ang * 3.1415926535 * 2);
  float ty = length * cos(ang * 3.1415926535 * 2);
  float dx = tx / ty;
  float y = -ty;
  float x = -tx;
  if (xy)
  {
    while (y <= ty)
    {
      for (int t = -width; t <= width; t++)
        Inject(m, round(x) + t, hf * round(y), energy);
      y += 1;
      x += dx;
    }
  } else {
    while (y <= ty)
    {
      for (int t = -width; t <= width; t++)
        Inject(m, round(y), hf * round(x) + t, energy);
      y += 1;
      x += dx;
    }
  }
  angle += 0.0012;
}

void Rectangle::DrawStochastic(Matrix& m, int n)
{
  float de = energy / ratio;
  for (int i = 0; i < n; i++)
  {
    float dist1 = RandFloat(-length, length);
    float dist2 = RandFloat(-width, width);
    float dx = sin(angle * 3.1415926535 * 2);
    float dy = cos(angle * 3.1415926535 * 2);
    float x = dist1 * dx + dist2 * dy;
    float y = dist1 * dy - dist2 * dx;
    Inject(m, y, x, de);
  }
}
*/
void GaussianCircle::DrawFull(Matrix& m)
{
  float r2 = radius * radius;
  for (int i = -radius; i <= radius; i++)
    for (int j = -radius; j <= radius; j++)
    {
      float d = i * i + j * j;
      if (d <= r2) Inject(m, i, j, energy * exp(3*-d/r2));
    }
}
   
void GaussianCircle::DrawStochastic(Matrix& m, int n)
{
  float r2 = radius * radius;
  float de = energy / ratio;
  int r, c, d;
  for (int t = 0; t < n; t++)
  {
    do {
      r = RandInt(-radius, radius);
      c = RandInt(-radius, radius);
      d = r * r + c * c;
    } while (d >= r2);
    Inject(m, r, c, de * exp(3*-d/r2));
  }
}


