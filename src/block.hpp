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

class PushableBlock : public Block
{
public:

    virtual void Interact(Creat& c);
    virtual void WasPushed(Creat& c);
};

class StaticTrap : public Block
{
public:
    StaticTrap();

    void Interact(Creat &c);
};

class ActiveTrap : public PushableBlock
{
public:
    ActiveTrap();

public:
    virtual void Update();
};

class RewardBlock : public PushableBlock
{
public:
    int reward;
    RewardBlock();

    virtual void WasPushed(Creat& c);
};

class SkinnerBlock : public Block
{
public:
    int reward;
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
