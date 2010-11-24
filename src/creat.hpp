#ifndef CREAT_HPP
#define CREAT_HPP

#include <iostream>
#include <list>

#include "occupant.hpp"
#include "matrix.hpp"

enum CreatAction
{
    ActionNone = 0,
    ActionForward,
    ActionLeft,
    ActionRight,
    ActionReproduce,
    NumberActions, // dummy for bookkeeping purposes
};

class Grid;
class Creat;
typedef void (Creat::*CreatFunc)();

struct MutationProfile
{
    bool colordrift;
    float proba;
    float probb;
    float probc;
    float noise;
    float scale;      
};

class LineageNode
{
public:
    Pos pos;
    float value;
    int timestep;
    int refs;
    LineageNode* prev;

    LineageNode(LineageNode* p);
    void Increment();
    void Decrement();
};

class Creat : public Occupant
{
public:
    static const int extinputs = 6;
    static const int intinputs = 4;
    static const int inputs = extinputs + intinputs;
    static const int hiddena = 2;
    static const int hiddenb = 2;
    static const int hidden = hiddena + hiddenb;
    static const int outputs = 4;
    static const int neurons = inputs + hidden + outputs;

    static bool lineages;
    
    static bool drawbirth;
    static bool drawoutline;
    static bool drawtrails;
    static bool drawspecial;
    static MutationProfile mprofile;

    static float actioncost[NumberActions];
    static CreatFunc actionlookup[NumberActions];
    static float initialenergy;
    static float initialmarker;
    static int maxage;
    static int steps;
    static int maxtraillen;
    static int nonzeroweights;
    static Matrix weightmask;
    static void Setup();
    static void SetupMutation();
    static void SetupMask(bool);
    static void SetupActions();

    int desired_id;
    int desirer_id;
    LineageNode* lineage;
    Matrix weights;
    Matrix state;
    Matrix state2;

    short action;
    short id;
    short age;
    short orient;
    bool possessed;
    bool alive;
    bool special;
    float energy;
    float marker;

    // MANAGEMENT
    Creat();
    Creat(const Creat& c); 
    void Reset();
    void SanityCheck();

    // LINEAGE HANDLING
    std::list<LineageNode> ReconstructLineage();
    void AddToLineage(Pos w);
    
    // OCCUPANT CODE
    void __Remove();
    void Interact(Creat& o);
    Pos Front(int offset=0);
    
    // UPDATE AND ACTION CODE
    void Step();
    void DoNothing();
    void MoveForward();
    void TurnLeft();
    void TurnRight();
    void Reproduce();
    void MutateBrain();
    void CopyBrain(Creat& parent);
    void BlendBrain(Creat& other);

    // INTERACTION CODE
    void Interaction(Creat& c);
    void ChooseMate(Creat* other);
    void TransferEnergy(Creat& other, float de);

    float Complexity();

    Creat* Peer(int id);
};

std::vector<Matrix> ReconstructBrains(std::list<LineageNode>& lineage, Matrix& initial, int T=0);
void ShuffleBrains(Matrix& a, Matrix& b);

#endif
