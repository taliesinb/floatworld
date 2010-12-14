#include "occupant.hpp"
#include "grid.hpp"

using namespace std;

RegisterAbstractClass(Occupant, None)
RegisterVar(Occupant, pos)
RegisterVar(Occupant, signature)
RegisterVar(Occupant, id)

RegisterQtHook(Occupant, signature, "color", FloatHook(-5.0, 5.0, 0.25));
RegisterQtHook(Occupant, solid, "solid", BoolHook());

Occupant::Occupant()
  : next(NULL), grid(NULL), pos(0,0), signature(0), id(-1), solid(true)
{
}

void Occupant::Interact(Creat&)
{
	
}

void Occupant::Reset()
{
}

void Occupant::__Remove()
{
	
}

void Occupant::AssignID()
{
    assert(grid);
    id = grid->next_id++;
}

void Occupant::Attach(Grid& g, Pos p)
{
    grid = &g;
    grid->occupant_list.push_back(this);
    Move(p);
    last_pos = p;
}

void Occupant::Update()
{

}

// remove an occupant
void Occupant::Remove()
{
    RemoveFromLL();
    grid->occupant_list.remove(this);
    DeleteQtHook();
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
    last_pos = pos;
    pos = pos2;
    Occupant*& cell = grid->OccupantAt(pos);
    next = cell;
    cell = this;
}

void Occupant::MoveRandom()
{
    Move(grid->RandomCell());
}

std::ostream& operator<<(std::ostream& s, Occupant*& o)
{
    s << *o;
    return s;
}

std::istream& operator>>(std::istream& s, Occupant*& o)
{
    o = dynamic_cast<Occupant*>(Class::Create(s));
    return s;
}

