#include "block.hpp"
#include "grid.hpp"
#include "shape.hpp"
#include "creat.hpp"

using namespace std;

RegisterClass(Block, Occupant);
RegisterVar(Block, fill);
RegisterVar(Block, hue);

RegisterClass(MoveableBlock, Block);

RegisterClass(SkinnerBlock, Block);
RegisterVar(SkinnerBlock, touch_count);
RegisterVar(SkinnerBlock, o_touch_count);
RegisterVar(SkinnerBlock, phase);

RegisterClass(PhasedSkinnerBlock, SkinnerBlock);
RegisterVar(PhasedSkinnerBlock, period);
RegisterVar(PhasedSkinnerBlock, phase);

RegisterClass(CloneBlock, Block);
RegisterVar(CloneBlock, period);
RegisterVar(CloneBlock, creat_id);

Block::Block()
  : Occupant(-1.5),
    fill(false),
    hue(0.5)
{
}


EnergyBlock::EnergyBlock()
  : de(0.5)
{
}

EnergyBlock::EnergyBlock(float _de)
  : de(_de)
{
}


void EnergyBlock::Update()
{
  if (grid->timestep % 5)
  {
    GaussianCircle spot;
    spot.pos = pos;
    spot.radius = 5;
    spot.energy = de;
    spot.Draw(grid->energy);
  }
}

void Block::Update()
{
  grid->energy(pos) = 0;
}

void Block::__Remove()
{
	delete this;
}

void MoveableBlock::Interact(Creat& c)
{
  Pos p = (pos + Pos(c.orient)).Wrap(grid->rows, grid->cols);
  Occupant* occ = grid->OccupantAt(p);
  
  if (dynamic_cast<MoveableBlock*>(occ)) occ->Interact(c);

  if (grid->OccupantAt(p) == NULL)
  {
    Move(p);
    c.energy += 6;
  }
}

SkinnerBlock::SkinnerBlock()
  : touch_count(0),
    o_touch_count(0)
{
  hue = 0.1;
}

void SkinnerBlock::Interact(Creat&)
{
  if (touch_count++ >= 8)
  {
    touch_count = 0;
    GaussianCircle spot;
    spot.pos = pos;
    spot.radius = 5;
    spot.energy = 15;
    spot.Draw(grid->energy);
  }
}


void SkinnerBlock::Update()
{
  fill = touch_count != o_touch_count;
  o_touch_count = touch_count;
  grid->energy(pos) = 0;
}

PhasedSkinnerBlock::PhasedSkinnerBlock()
  : period(20)
{
}

void PhasedSkinnerBlock::Update()
{
  phase = ((grid->timestep + pos.row + pos.col) / period) % 2;
  
  grid->energy(pos) = 0;
  fill = touch_count != o_touch_count;
  o_touch_count = touch_count;

  signature = phase ? -1.0 : -2.0;
  hue = phase ? 0.1 : 0.4;
}

void PhasedSkinnerBlock::Interact(Creat& c)
{
  if (phase and touch_count++ >= 8)
  {
    touch_count = 0;
    GaussianCircle spot;
    spot.pos = pos;
    spot.radius = 5;
    spot.energy = 30;
    spot.threshold = 30;
    spot.Draw(grid->energy);
  }
  if (!phase)
  {
    c.energy -= 50;
  }
}

CloneBlock::CloneBlock()
  : period(20),
    creat_id(-1)
{
  hue = 0.7;
}

void CloneBlock::Interact(Creat& c)
{
	creat_id = c.id;
}

void CloneBlock::Update()
{
  grid->energy(pos) = 0;
  Creat* creat = Peer(creat_id);
	if (grid->timestep % period == 0 && creat && creat->alive)
  {
    Creat& child = grid->_AddCreat(grid->FairCell(), RandInt(3));
    child.CopyBrain(*creat);
    if (grid->mutation) child.MutateBrain();
    fill = true;
    creat = NULL;
  } else fill = false;
}




