#include "shape.hpp"

RegisterAbstractClass(Shape, Occupant);
RegisterVar(Shape, threshold);
RegisterVar(Shape, energy);
RegisterVar(Shape, ratio);
RegisterVar(Shape, p_jump);

RegisterBinding(Shape, threshold, "Emax", -50,50,0.25);
RegisterBinding(Shape, energy, "Einc", -5,5,0.1);
RegisterBinding(Shape, ratio,  "fill", 0,1,0.1);
RegisterBinding(Shape, p_jump, "pjump", 0, 1, 0.001);

RegisterClass(EnergyDisk, Shape);
RegisterVar(EnergyDisk, radius);
RegisterBinding(EnergyDisk, radius, "radius", 0, 100);

RegisterClass(EnergyGaussian, EnergyDisk);

RegisterClass(EnergyAnnalus, Shape);
RegisterVar(EnergyAnnalus, radius1);
RegisterVar(EnergyAnnalus, radius2);

RegisterBinding(EnergyAnnalus, radius1, "inner radius", 0, 20, 1);
RegisterBinding(EnergyAnnalus, radius2, "outer radius", 0, 20, 1);

RegisterClass(EnergyRectangle, Shape);
RegisterVar(EnergyRectangle, width);
RegisterVar(EnergyRectangle, length);
RegisterVar(EnergyRectangle, angle);

RegisterBinding(EnergyRectangle, width, "width", 0, 30);
RegisterBinding(EnergyRectangle, length, "length", 0, 30);
RegisterBinding(EnergyRectangle, angle, "angle", 0, 1, 0.05);

Shape::Shape()
    : threshold(5),
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
    Draw(world->energy);
    if (rng.Bool(p_jump))
        MoveRandom();
}

EnergyDisk::EnergyDisk()
{
    radius = 10;
}

float EnergyDisk::Area()
{
    return 3.141519 * radius * radius;
}

void EnergyDisk::DrawFull(Matrix& m)
{
    float r2 = radius * radius;
    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
            if (abs(i) + abs(j) < radius || (i * i) + (j * j) < r2)
                Inject(m, i, j, energy);
}


void EnergyDisk::DrawStochastic(Matrix& m, int n)
{
    float r2 = radius * radius;
    float de = energy / ratio;
    for (int t = 0; t < n; t++)
    {
        int r, c;
        do {
            r = rng.Integer(-radius, radius);
            c = rng.Integer(-radius, radius);
        } while (r * r + c * c > r2);
        Inject(m, r, c, de);
    }
}

EnergyAnnalus::EnergyAnnalus()
    : radius1(5),
    radius2(10)
{

}

float EnergyAnnalus::Area()
{
    return 3.14159 * (radius2 * radius2 - radius1 * radius1);
}

void EnergyAnnalus::DrawFull(Matrix& m)
{
    float ir2 = radius1 * radius1;
    float or2 = radius2 * radius2;
    for (int i = -radius2; i <= radius2; i++)
        for (int j = -radius2; j <= radius2; j++) {
        float d = i * i + j * j;
        if (d >= ir2 && d < or2)
            Inject(m, i, j, energy);
    }
}

void EnergyAnnalus::DrawStochastic(Matrix& m, int n)
{
    float ir2 = radius1 * radius1;
    float or2 = radius2 * radius2;
    float de = energy / ratio;
    for (int t = 0; t < n; t++)
    {
        int r, c, d;
        do {
            r = rng.Sign() * rng.Integer(radius1, radius2);
            c = rng.Sign() * rng.Integer(radius1, radius2);
            d = r * r + c * c;
        } while (d < ir2 || d > or2);
        Inject(m, r, c, de);
    }
}

EnergyRectangle::EnergyRectangle()
    : width(3), length(20), angle(0)
{  
}

float EnergyRectangle::Area()
{
    return length * width;
}

void EnergyRectangle::DrawFull(Matrix& m)
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
    //angle += 0.0012;
}

void EnergyRectangle::DrawStochastic(Matrix& m, int n)
{
    float de = energy / ratio;
    for (int i = 0; i < n; i++)
    {
        float dist1 = rng.Float(-length, length);
        float dist2 = rng.Float(-width, width);
        float dx = sin(angle * 3.1415926535 * 2);
        float dy = cos(angle * 3.1415926535 * 2);
        float x = dist1 * dx + dist2 * dy;
        float y = dist1 * dy - dist2 * dx;
        Inject(m, y, x, de);
    }
}

void EnergyGaussian::DrawFull(Matrix& m)
{
    float r2 = radius * radius;
    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
        {
        float d = i * i + j * j;
        if (d <= r2) Inject(m, i, j, energy * exp(3*-d/r2));
    }
}

void EnergyGaussian::DrawStochastic(Matrix& m, int n)
{
    float r2 = radius * radius;
    float de = energy / ratio;
    int r, c, d;
    for (int t = 0; t < n; t++)
    {
        do {
            r = rng.Integer(-radius, radius);
            c = rng.Integer(-radius, radius);
            d = r * r + c * c;
        } while (d >= r2);
        Inject(m, r, c, de * exp(3*-d/r2));
    }
}


