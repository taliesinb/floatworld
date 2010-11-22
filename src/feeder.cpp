#include <assert.h>
#include <math.h>
#include <iostream>
#include <string>

#include "feeder.hpp"
#include "grid.hpp"
#include "shape.hpp"

using namespace std;



Feeder::Feeder() : grid(NULL),
                   energy(NULL),
                   cache(NULL),
                   RLEcache(NULL),
                   maxdensity(20),
                   useRLE(false),
                   cacheable(true),
                   rows(0),
                   cols(0)
{
}

Feeder::~Feeder()
{
  if (cache) delete cache;
  if (RLEcache) delete [] RLEcache;
}


void Feeder::Resize()
{
  rows = grid->rows;
  cols = grid->cols;
  if (cache)
  {
    DeleteCache();
    UseCache(useRLE);
  }
  InternalResize();
}

void Feeder::InternalResize()
{
}

void Feeder::SetGrid(Grid* g)
{
  grid = g;
  energy = &g->energy;
  rows = g->rows;
  cols = g->cols;
}

void Feeder::DeleteCache()
{
  if (!cacheable) return;

  if (cache) delete cache;
  cache = NULL;

  if (RLEcache) delete [] RLEcache;
  RLEcache = NULL;
}

void Feeder::UseCache(bool rle)
{
  if (!cacheable) return;
   
  if (!cache) cache = new Matrix(rows, cols);

  if (rle) RLEcache = new RLEpair[rows * cols];

  useRLE = rle;

  Recache();
}

void Feeder::Recache()
{
  if (cache == NULL || !cacheable) return;
   
  cache->SetZero();

  Matrix* temp1 = energy;
  energy = cache;
  InternalFeedStep();
  energy = temp1;

  if (useRLE)
  {
    EncodeRLE(RLEcache, *cache);
  }
}

Pos Feeder::GridPos(float r, float c)
{
  return Pos(GridRow(r), GridCol(c));
}

int Feeder::GridRow(float r)
{
  return Mod(round(r * (rows - 1)), rows);
}

int Feeder::GridCol(float c)
{
  return Mod(round(c * (cols - 1)), cols);
}

/*
  inline void Feeder::FeedCell(int r, int c, float inj)
  {
  float &e = energy->Get(r,c);
//   float old = e;
if (e < maxdensity) e += inj;
//   e += inj;
//   if (e > maxdensity) e = maxdensity;
//   totalenergy += (e - old);
}
*/

void Feeder::InitialFeed()
{
  energy->SetZero();
  for (int i = 0; i < 15; i++) FeedStep();
}

void Feeder::Feed()
{
  Step();
  FeedStep();
}

void Feeder::Step()
{
}

void Feeder::InternalFeedStep()
{
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      FeedCell(i, j, 0.1);
}

void Feeder::SaveState(std::ostream& os)
{
}

void Feeder::LoadState(std::istream& is)
{
}

void Feeder::FeedStep()
{
  if (cache && cacheable)
  {
    float* data1 = cache->data;
    float* data2 = energy->data;
    if (useRLE)
    {
      int pos = 0, i = 0;
      while (pos < rows * cols)
      {
        float v = RLEcache[i].value;
        int l = RLEcache[i].length;
        i++;
        if (v)
        {
          while (l--)
          {
            if (data2[pos] < maxdensity) data2[pos] += v;
            pos++;
          }
        } else pos += l;
      }
    } else {
      for (int i = 0; i < rows * cols; i++)
        if (data2[i] < maxdensity) data2[i] += data1[i];
    }
  } else
    InternalFeedStep();
}

ShapeFeeder::ShapeFeeder()
  : moveprob(0.01)
{
}

ShapeFeeder::~ShapeFeeder()
{
  RemoveAllShapes();
}

void ShapeFeeder::SaveState(std::ostream& os)
{
	os << shapes.size() << endl;

  for_iterate(it, shapes)
  {
    os << (*it)->Name() << endl;
    (*it)->Write(os);
  }
}

void ShapeFeeder::LoadState(std::istream& is)
{
  int num_shapes;

  RemoveAllShapes();

  is >> num_shapes;
  for (int i = 0; i < num_shapes; i++)
  {
    Shape* shape = dynamic_cast<Shape*>(MetaClass::Create(is));
    AddShape(shape);
  }  
}

void ShapeFeeder::AddShape(Shape* shape)
{
  shapes.push_back(shape);
}

void ShapeFeeder::RemoveAllShapes()
{
	for_iterate(it, shapes)
    delete *it;
  shapes.clear();
}

void ShapeFeeder::SetRatio(float ratio)
{
  for_iterate(it, shapes) 
    (*it)->ratio = ratio;
  Recache();
}


void ShapeFeeder::InternalFeedStep()
{
  for_iterate(it, shapes)
  {
    (*it)->Draw(*energy);
  }
}

void ShapeFeeder::Step()
{
  if (moveprob == 0) return;
   
  bool valid = true;
  for_iterate(it, shapes)
  {
    if (RandFloat() < moveprob)
    {
      (*it)->RandomPosition();
      valid = false;
    }
  }
  if (!valid) Recache();
}

