#include "occupant.hpp"
#include "grid.hpp"

using namespace std;

RegisterAbstractClass(Occupant, None)
RegisterVar(Occupant, pos)
RegisterVar(Occupant, signature)

Occupant::Occupant(int sig)
  : next(NULL), grid(NULL), pos(0,0), signature(sig)
{
}

void Occupant::Interact(Creat&)
{
	
}

void Occupant::__Remove()
{
	
}

void Occupant::Place(Grid& g, Pos p)
{
    grid = &g;
    pos = p;
    Place();
    grid->occupant_list.push_back(this);
}

// add an occupant for the first time
void Occupant::Place()
{
    Occupant*& cell = grid->OccupantAt(pos);
    next = cell;
    cell = this;
}

void Occupant::Update()
{

}

// remove an occupant
void Occupant::Remove()
{
    RemoveFromLL();
    grid->occupant_list.remove(this);
    __Remove();
}

// remove an occupant but only from the internal LL
void Occupant::RemoveFromLL()
{
    Occupant** cell = &(grid->OccupantAt(pos));
    while (*cell)
    {
        if (*cell == this)
        {
            *cell = next;
            next = NULL;
            return;
        }
        cell = &((*cell)->next);
    }
}

// move an occupant from an existing position
void Occupant::Move(Pos pos2)
{
    RemoveFromLL();
    pos = pos2;
    Place();
}

void Occupant::MoveRandom()
{
    RemoveFromLL();
    pos = grid->RandomCell();
    Place();
}

Creat* Occupant::Peer(int id)
{
    return (id < 0) ? NULL : grid->LookupCreatByID(id);
}

std::ostream& operator<<(std::ostream& s, Occupant* o)
{
    s << *o;
}

std::istream& operator<<(std::istream& i, Occupant* o)
{

}

