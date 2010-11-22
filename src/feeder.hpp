#ifndef FEEDER_HPP
#define FEEDER_HPP

#include "matrix.hpp"
#include "misc.hpp"
#include <vector>
#include <iostream>

using namespace std;

/*

Feeder is the class that abstracts away the details of how a grid
is fed with energy. A feeder is "attached" to a grid using
Grid::AttachFeeder. It has pointers to the grid and the grid's
energy matrix. The grid calls Feeder::FeedStep every timestep to
perform feeding.

Derived classes override InternalFeedStep to specify their own
feeding algorithm. This function might be called multiple times
per step, or only every few steps if the feeding pattern does not
change often, so there is a seperate Step member function that is
only called once per timestep that should handle management
rather than actual feeding.

Two forms of caching are provided to improve performance. If the
feeding pattern does not change often, it can be stored in a
second energy matrix. In this case, Step should call Recache when
the pattern has changed. FeedStep will automatically use the
cache matrix without recalculating the pattern using
InternalFeedStep.

The second form of caching is RLE compression. Here, a cache
matrix is still used, but RLE compression is performed on when
Recache is called. FeedStep then automatically skips 0s in the
feeding pattern, which might improve performance for sparse
patterns.

Derived classes must set cacheable in their constructors if and
only if they have the correct Recache-calling semantics. Calling
Recache if caching is not enabled has no effect. The user of the
derived class ultimately chooses whether to use caching (and
whether it should be RLE) by calling Feeder::UseCache.

Grids can be dynamically resized, and any attached feeders must
update themselves if necessary. Grid::Resize calls Feeder::Resize
which calls the virtual Feeder::InternalResize which implements
any special code for derived classes.

Grid states can be saved to and loaded from files. The attached
feeder must likewise save and load its state from the given
iostreams, by overriding the provided Feeder::{SaveState,
LoadState} virtual functions.

Various primitive "drawing" operations can be used to feed energy
to the grid. Each drawing operation is represented by a Shape
object, which implements ordinary and monte-carlo versions of the
specified shape. 
*/

class Grid;

class Feeder
{
  public:
    Grid* grid;
    Matrix* energy;
    float maxdensity;

    Matrix* cache;
    RLEpair* RLEcache;
    bool cacheable;      
    bool useRLE;
      
    int rows;
    int cols;

    Feeder();
    ~Feeder();
    void Resize();
    virtual void InternalResize();

    Pos GridPos(float r, float c);
    int GridRow(float r);
    int GridCol(float c);

    void SetGrid(Grid* g);

    void UseCache(bool rle=false);
    void Recache();
    void DeleteCache();

    void FeedStep();
    void Feed();
    virtual void Step();
    void InitialFeed();
    virtual void InternalFeedStep();

    virtual void SaveState(std::ostream& os);
    virtual void LoadState(std::istream& is);

    // Primitives:
    void FeedCell(int r, int c, float inj)
    {
      float &e = (*energy)(r,c);
      if (fabs(e + inj) < maxdensity) e += inj;
    }
};

class Shape;
class ShapeFeeder : public Feeder
{
  public:

    typedef vector<Shape*> ShapeVec;
    ShapeVec shapes;
    float moveprob;

    ShapeFeeder();
    ~ShapeFeeder();
    void SaveState(std::ostream& os);
    void LoadState(std::istream& is);

    void SetRatio(float r);
    void AddShape(Shape* shape);
    void RemoveAllShapes();

    void InternalFeedStep();
    void Step();
};

#endif
