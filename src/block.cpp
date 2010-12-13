#include "block.hpp"
#include "grid.hpp"
#include "shape.hpp"
#include "creat.hpp"

using namespace std;

RegisterClass(Block, Occupant);

RegisterClass(RewardBlock, Block);
RegisterVar(RewardBlock, reward);
RegisterQtHook(RewardBlock, reward, "ereward", IntegerHook(-100,100));

RegisterClass(MoveableBlock, RewardBlock);

RegisterClass(SkinnerBlock, RewardBlock);
RegisterVar(SkinnerBlock, touch_count);
RegisterVar(SkinnerBlock, threshold);
RegisterVar(SkinnerBlock, radius);

RegisterQtHook(SkinnerBlock, touch_count, "count", IntegerHook(0,100));
RegisterQtHook(SkinnerBlock, threshold, "threshold", IntegerHook(0,20));
RegisterQtHook(SkinnerBlock, radius, "radius", IntegerHook(1,20));

RegisterClass(PhasedSkinnerBlock, SkinnerBlock);
RegisterVar(PhasedSkinnerBlock, period);
RegisterVar(PhasedSkinnerBlock, phase);
RegisterQtHook(PhasedSkinnerBlock, period, "period", IntegerHook(1,20));
RegisterQtHook(PhasedSkinnerBlock, phase, "phase", BoolHook());

Block::Block()
{
    draw_filled = false;
    draw_hue = 0.5;
    signature = -2;
}

void Block::Update()
{
    grid->energy(pos) = 0;
}

RewardBlock::RewardBlock()
{
    reward = 10;
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
{
    touch_count = _touch_count = 0;
    threshold = 8;
    reward = 20;
    radius = 5;
    draw_hue = 0.1;
}

void SkinnerBlock::Interact(Creat&)
{
    if (touch_count++ >= threshold)
    {
        touch_count = 0;
        Circle spot;
        spot.pos = pos;
        spot.radius = radius;
        spot.energy = reward;
        spot.Draw(grid->energy);
    }
}


void SkinnerBlock::Update()
{
    draw_filled = (touch_count != _touch_count);
    _touch_count = touch_count;
    grid->energy(pos) = 0;
}

PhasedSkinnerBlock::PhasedSkinnerBlock()
{
    period = 8;
}

void PhasedSkinnerBlock::Update()
{
    SkinnerBlock::Update();

    phase = ((grid->timestep + pos.row + pos.col) / period) % 2;
  
    signature = phase ? -1.0 : -2.0;
    draw_hue = phase ? 0.1 : 0.4;
}

void PhasedSkinnerBlock::Interact(Creat& c)
{
    if (phase)
        SkinnerBlock::Interact(c);
    else
        c.energy -= 50;
}
