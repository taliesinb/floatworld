#ifndef OCCUPANT_HPP
#define OCCUPANT_HPP

#include "pos.hpp"
#include "metaclass.hpp"

class Grid;
class Creat;

class Occupant : public Class
{
  public:

    Occupant(int sig);
    
    Occupant* next; // next occupant on this grid position
    Grid* grid;
    Pos pos;
    int signature;

    virtual void Interact(Creat& c);
    virtual void Update();

    virtual void __Remove();

    void Place(Grid& grid, Pos pos);
    void Place();
    void Remove();
    void RemoveFromLL();
    void Move(Pos p);
    void MoveRandom();
    
    Creat* Peer(int id);
};

std::ostream& operator<<(std::ostream& s, Occupant* o);
std::istream& operator<<(std::istream& i, Occupant* o);

#endif
