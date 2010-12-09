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
    static const int hidden = 4;
    static const int outputs = 4;
    static const int neurons = inputs + hidden + outputs;

    static int global_id;

    static void Setup();

    int desired_id;
    int desirer_id;
    LineageNode* lineage;
    Matrix weights;
    Matrix state;
    Matrix state2;

    unsigned long int fingerprint;
    int action;
    int age;
    int orient;
    bool possessed;
    bool alive;
    float energy;
    float marker;

    // MANAGEMENT
    Creat();
    Creat(const Creat& c);
    void Reset();

    // LINEAGE HANDLING
    std::list<LineageNode> ReconstructLineage();
    void AddToLineage(Pos w);
    
    // OCCUPANT CODE
    void __Remove();
    void Interact(Creat& o);
    Pos Front(int offset=0);
    
    // UPDATE AND ACTION CODE
    void CheckSanity(const char* str);
    void Update();
    void Step();
    void DoNothing();
    void MoveForward();
    void TurnLeft();
    void TurnRight();
    void Reproduce();
    void MutateBrain();
    void CopyBrain(Creat& parent);
    void BlendBrain(Creat& other);
    Pos SelectRandomWeight();

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
