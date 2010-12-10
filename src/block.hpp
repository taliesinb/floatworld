#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <iostream>

#include "occupant.hpp"
#include "pos.hpp"

class Block : public Occupant
{
public:

    Block();

    bool draw_filled;
    float draw_hue;

    void Update();
};

class RewardBlock : public Block
{
public:
    RewardBlock();

    int reward;
};

class MoveableBlock : public RewardBlock
{
public:

    void Interact(Creat& c);
};

class SkinnerBlock : public RewardBlock
{
public:
    int threshold;
    int touch_count;
    int _touch_count;
    int radius;

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

    void Interact(Creat& c);
    void Update();
};

#endif
