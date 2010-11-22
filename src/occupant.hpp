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

    void Place();
    void Remove();
    void RemoveFromLL();
    void Move(Pos p);
    
    Creat* Peer(int id);
};



#endif
