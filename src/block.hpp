#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <iostream>

#include "occupant.hpp"
#include "pos.hpp"

class Block : public Occupant
{
public:

    Block();

    bool fill;
    float hue;

    void __Remove();
    void Update();
};

class MoveableBlock : public Block
{
public:

    void Interact(Creat& c);
};

class EnergyBlock : public Block
{
public:
    EnergyBlock();
    EnergyBlock(float _de);

    float de;

    void Update();
};

class SkinnerBlock : public Block
{
public:
    int touch_count;
    int o_touch_count;
    int phase;

    SkinnerBlock();

    void Interact(Creat& c);
    void Update();
};

class PhasedSkinnerBlock : public SkinnerBlock
{
public:

    int period;
    bool phase;

    PhasedSkinnerBlock();

    bool Phase();
    void Interact(Creat& c);
    void Update();
};

class CloneBlock : public Block
{
public:

    int period;
    int creat_id;

    CloneBlock();

    void Interact(Creat& c);
    void Update();
};

#endif
