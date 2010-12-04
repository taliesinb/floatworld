#include "occupant.hpp"
#include "grid.hpp"

using namespace std;

RegisterAbstractClass(Occupant, None)
RegisterVar(Occupant, pos)
RegisterVar(Occupant, signature)
RegisterVar(Occupant, id)

Occupant::Occupant()
  : next(NULL), grid(NULL), pos(0,0), signature(0), id(-1)
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
    cout << "Attaching " << reinterpret_cast<long int>(this) << " with id " << id << endl;
    Move(p);
}

void Occupant::Update()
{

}

// remove an occupant
void Occupant::Remove()
{
    RemoveFromLL();
    grid->occupant_list.remove(this);
    cout << "Removing occupant " << id << "|" << endl;
    cout << "Remaining occupants: [";
    for_iterate(it, grid->occupant_list)
    {
        cout << (*it)->id << " ";
    }
    cout << "]" << endl;

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
    o = dynamic_cast<Occupant*>(MetaClass::Create(s));
    return s;
}

