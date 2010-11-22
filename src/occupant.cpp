#include "occupant.hpp"
#include "grid.hpp"

using namespace std;

RegisterAbstractClass(Occupant, None)
RegisterVar(Occupant, pos)
RegisterVar(Occupant, signature)

Occupant::Occupant(float sig)
  : grid(NULL),
    signature(sig)
{
}

void Occupant::Interact(Creat& o)
{
	
}

void Occupant::__Remove()
{
	
}

void Occupant::Place()
{
  grid->OccupantAt(pos) = this;
}

void Occupant::Update()
{
	
}

void Occupant::Remove()
{
  grid->OccupantAt(pos) = NULL;
  __Remove();
}

void Occupant::Move(Pos p)
{
  grid->OccupantAt(pos) = NULL;
  pos = p;
  grid->OccupantAt(pos) = this;
}

Creat* Occupant::Peer(int id)
{
	return (id < 0) ? NULL : &grid->creats[id];
}



