#ifndef OCCUPANT_HPP
#define OCCUPANT_HPP

#include "pos.hpp"
#include "metaclass.hpp"

class Grid;
class Creat;

class Occupant : public Object
{
  public:

    Occupant();
    
    Occupant* next; // next occupant on this grid position
    Grid* grid;
    Pos pos;
    short signature;
    short id;

    virtual void Interact(Creat& c);
    virtual void Update();

    virtual void Reset();
    virtual void __Remove();
    void AssignID();

    void Attach(Grid& grid, Pos pos);
    void Remove();
    void RemoveFromLL();
    void Move(Pos p);
    void MoveRandom();
};

std::ostream& operator<<(std::ostream& s, Occupant*& o);
std::istream& operator>>(std::istream& i, Occupant*& o);

#endif
