#ifndef CREAT_HPP
#define CREAT_HPP

#include <iostream>
#include <list>

#include "world.hpp"

enum CreatAction
{
    ActionNone = 0,
    ActionForward,
    ActionLeft,
    ActionRight,
    ActionReproduce,
    NumberActions, // dummy for bookkeeping purposes
};

class World;
class Creat;

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
    static const int num_ext_inputs = 8;
    static const int num_int_inputs = 4;
    static const int num_inputs = num_ext_inputs + num_int_inputs;
    static const int num_hidden = 4;
    static const int num_outputs = 4;
    static const int num_neurons = num_inputs + num_hidden + num_outputs;

    static const int off_ext_inputs = 0;
    static const int off_int_inputs = num_ext_inputs;
    static const int off_hidden = num_inputs;
    static const int off_outputs = off_hidden + num_hidden;

    static int global_id;

    int desired_id;
    int desirer_id;
    LineageNode* lineage;
    Matrix weights;
    Matrix state;
    Matrix state2;

    unsigned long int fingerprint;
    int action;
    int age;
    int max_age;
    int orient;
    int last_orient;
    int interaction_count;
    int children;
    bool alive;
    bool interacted;
    float energy;
    float marker;

    // MANAGEMENT
    Creat();
    Creat(const Creat& c);
    void Reset();
    void Die();

    // OBJECT CODE
    void HookWasChanged();

    // LINEAGE HANDLING
    std::list<LineageNode> ReconstructLineage();
    void AddToLineage(Pos w);
    
    // OCCUPANT CODE
    void __Remove();
    void Interact(Creat& o);
    Pos Front(int offset=0, int dist=1);
    
    // UPDATE AND ACTION CODE
    void CheckSanity(const char* str);
    void Update();
    void UpdateBrain();
    void UpdateInputs();
    void DoNothing();
    void MoveForward();
    void TurnLeft();
    void TurnRight();
    void Reproduce();
    void MutateBrain();
    void CopyBrain(Creat& parent);
    void BlendBrain(Creat& other);
    void ShuffleBrain(Creat& other);
    Pos SelectRandomWeight();

    // INTERACTION CODE
    void Interaction(Creat& c);
    void ChooseMate(Creat* other);
    void TransferEnergy(Creat& other, float de);

    float Complexity();

    Creat* Peer(int id);
};

#endif
