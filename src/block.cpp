#include "block.hpp"
#include "shape.hpp"

using namespace std;

RegisterClass(Block, Occupant);

RegisterClass(PushableBlock, Block);

RegisterClass(RewardBlock, PushableBlock);
RegisterVar(RewardBlock, reward);
RegisterBinding(RewardBlock, reward, "reward", -100, 100);

RegisterClass(StaticTrap, Trap);
RegisterClass(ActiveTrap, PushableBlock);

RegisterClass(SkinnerBlock, Block);
RegisterVar(SkinnerBlock, touch_count);
RegisterVar(SkinnerBlock, threshold);
RegisterVar(SkinnerBlock, radius);
RegisterVar(SkinnerBlock, reward);

RegisterBinding(SkinnerBlock, touch_count, "count", 0, 100);
RegisterBinding(SkinnerBlock, threshold, "threshold", 0, 20);
RegisterBinding(SkinnerBlock, radius, "radius", 1, 20);
RegisterBinding(SkinnerBlock, reward, "reward", 1, 20);

RegisterClass(PhasedSkinnerBlock, SkinnerBlock);
RegisterVar(PhasedSkinnerBlock, period);
RegisterVar(PhasedSkinnerBlock, phase);
RegisterBinding(PhasedSkinnerBlock, period, "period", 1, 20);
RegisterBinding(PhasedSkinnerBlock, phase, "phase");

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

void PushableBlock::Interact(Creat& c)
{
    Pos p = grid->Wrap(pos + Pos(c.orient));
    Occupant* occ = grid->OccupantAt(p);
  
    if (dynamic_cast<PushableBlock*>(occ)) occ->Interact(c);

    if (grid->OccupantAt(p) == NULL)
    {
        Move(p);
        WasPushed(c);
    }
}

void PushableBlock::WasPushed(Creat &c)
{
}

RewardBlock::RewardBlock()
{
    reward = 10;
}

void RewardBlock::WasPushed(Creat &c)
{
    c.energy += reward;
}

StaticTrap::StaticTrap()
{
    signature = -2.0;
    draw_hue = 0.0;
}

void StaticTrap::Interact(Creat &c)
{
    c.alive = false;
}

ActiveTrap::ActiveTrap()
{
    signature = -2.0;
    draw_hue = 0.9;
}

void ActiveTrap::Update()
{
    Pos new_pos;
    for (int dir = 0; dir < 4; dir++)
    {
        new_pos = grid->Wrap(pos + Pos(dir));
        if (Creat* creat = grid->CreatAt(new_pos))
        {
            creat->alive = false;
            Move(new_pos);
            draw_filled = true;
            return;
        }
    }
    draw_filled = false;
    new_pos = grid->Wrap(pos + Pos(RandInt(0,3)));
    if (!grid->OccupantAt(new_pos)) Move(new_pos);
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
    draw_filled = (touch_count < _touch_count);
    _touch_count = touch_count;
    grid->energy(pos) = 0;
}

PhasedSkinnerBlock::PhasedSkinnerBlock()
{
    period = 8;
    phase = false;
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

