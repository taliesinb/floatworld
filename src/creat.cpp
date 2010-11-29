#include "creat.hpp"
#include "grid.hpp"

#include <fstream>

using namespace std;

RegisterClass(Creat, Occupant);
RegisterVar(Creat, desired_id);
RegisterVar(Creat, desirer_id);
RegisterVar(Creat, weights);
RegisterVar(Creat, state);
RegisterVar(Creat, action);
RegisterVar(Creat, age);
RegisterVar(Creat, orient);
RegisterVar(Creat, possessed);
RegisterVar(Creat, alive);
RegisterVar(Creat, special);
RegisterVar(Creat, energy);
RegisterVar(Creat, marker);

int Creat::steps = 0;
int Creat::maxage = 100;

float Creat::initialenergy = 2;
float Creat::initialmarker = 0.0;
float Creat::actioncost[NumberActions];
CreatFunc Creat::actionlookup[NumberActions];
Matrix Creat::weightmask(Creat::neurons, Creat::neurons);
MutationProfile Creat::mprofile;
bool Creat::lineages = false;
bool Creat::drawoutline = false;
bool Creat::drawbirth = true;
bool Creat::drawtrails = false;
bool Creat::drawspecial = false;

LineageNode::LineageNode(LineageNode* p)
    : pos(0,0), prev(p), refs(1)
{
}


void LineageNode::Increment()
{
    refs++;
}

void LineageNode::Decrement()
{
    refs--;
    if (refs <= 0) delete this;
}

void Creat::Setup()
{
    SetupMutation();
    SetupMask(true);
    SetupActions();
}

void Creat::SetupMutation()
{
    mprofile.color_drift = true;
    mprofile.mutation_prob = 0.0;
    mprofile.mutation_sd = 3.0;
}

void Creat::SetupMask(bool hid)
{
    int inl =  0;
    int inr =  Creat::inputs - 1;
    int hidl = Creat::inputs;
    int hidr = Creat::inputs + Creat::hidden - 1;
    int outl = hidr + 1;
    int outr = hidr + Creat::outputs;

    weightmask.SetZero();
    if (hid)
    {
        weightmask.SetSubMatrix(hidl, inl, hidr, inr, 1.0);
        weightmask.SetSubMatrix(outl, hidl, outr, hidr, 1.0);
        weightmask.SetSubMatrix(hidl, hidl, hidr, hidr, 1.0);
    }
    weightmask.SetSubMatrix(outl, inl, outr, inr, 1.0);
}

void Creat::SetupActions()
{
    actionlookup[ActionNone] = &Creat::DoNothing;
    actionlookup[ActionForward] = &Creat::MoveForward;
    actionlookup[ActionLeft] = &Creat::TurnLeft;
    actionlookup[ActionRight] = &Creat::TurnRight;
    actionlookup[ActionReproduce] = &Creat::Reproduce;
  
    actioncost[ActionNone] = 0;
    actioncost[ActionLeft] = 0;
    actioncost[ActionForward] = 1.0;
    actioncost[ActionRight] = 0;
    actioncost[ActionReproduce] = 60.0;
}  

Creat::Creat()
    : Occupant(1),
      weights(neurons, neurons),
      state(neurons, 1),
      state2(neurons, 1)
{
    Reset();
    weights.SetZero();
    state.SetZero();
    state2.SetZero();
    alive = false;
    pos.row = pos.col = 0;
}

Creat::Creat(const Creat& c)
    : Occupant(1),
      weights(1,1),
      state(1,1),
      state2(1,1)
{
    assert(0);
}

void Creat::Reset()
{
    state.SetZero();
    age = 0;
    action = ActionNone;
    possessed = false;
    alive = false;
    marker = initialmarker;
    energy = initialenergy;
    lineage = NULL;
    desired_id = -1;
    desirer_id = -1;
}


void Creat::AddToLineage(Pos w)
{
    LineageNode* node = new LineageNode(lineage);
    node->timestep = grid->timestep;
    node->value = weights(w);
    node->pos = w;
    lineage = node;
}

list<LineageNode> Creat::ReconstructLineage()
{
    list<LineageNode> stack;
    list<int> times;

    int t = lineage ? lineage->timestep + 1 : 0;
    int depth = 0;
    while (lineage)
    {
        if (lineage->timestep < t)
        {
            t = lineage->timestep;
            depth++;
            times.push_front(t);
        }
        stack.push_front(*lineage);
        lineage = lineage->prev;
    }
    cout << "Depth of lineage: " << depth << endl;

    ofstream os("lineage_times");
    for (list<int>::iterator it = times.begin(); it != times.end(); it++)
    {
        os << *it << endl;
    }
  
    return stack;
}

Pos SelectRandomWeight()
{
    int j, k;
    do { 
        j = RandInt(Creat::neurons-1);
        k = RandInt(Creat::neurons-1);
    } while (Creat::weightmask(j,k) == 0.0);
    return Pos(j,k);
}

void Creat::Reproduce()
{
    Pos front = (Front() + Pos(RandInt(-3,3), RandInt(-3,3))).Wrap(grid->rows, grid->cols);

    if (grid->OccupantAt(front)) return;
  
    Creat& child = grid->_AddCreat(front, Mod(orient+RandSign(),4));
    child.CopyBrain(*this);
    child.MutateBrain();

    float excess = state(inputs + hidden + ActionReproduce - 1) - 0.8;
    TransferEnergy(child, excess * 10);

    grid->births++;
}

void Creat::MoveForward()
{ 
    Pos front = Front();

    if (Occupant* other = grid->OccupantAt(front))
    {
        other->Interact(*this);
        special = true;
    } else 
    {
        Move(front);
        float de = grid->energy(front);
        energy += de;
        grid->energy(pos) = grid->pathenergy / 2.0;
    }
}

void Creat::DoNothing()
{
}

void Creat::TransferEnergy(Creat& other, float de)
{
    if (de < 0) other.TransferEnergy(*this, -de);
    else {
        de = Min(energy, de);
        energy -= de;
        other.energy += de * 0.75;
    }
}

void Creat::Interact(Creat& creat)
{
    creat.Interaction(*this);
}

void Creat::Interaction(Creat& other)
{
    switch (grid->interaction)
    {
        case NoInteraction:
            break;

        case Wastage:
            energy -= 20;
            break;
      
        case Parasitism:
            energy -= 20;
            other.weights = weights;
            break;
      
        case Predation: 
            other.TransferEnergy(*this, RandFloat(50,100));
            break;

        case Cooperation: 
            energy += 0;
            other.energy += 20;
            //energy(pos) = -10;
            //energy(other.pos) = -10;
            //energy(other.pos) = 20;
            break;

        case GeneExchange: 
            energy -= 4;
            if (energy > 0) ShuffleBrains(weights, other.weights);
            break;

        case GeneGive:
            energy -= 5;
            ChooseMate(this);
            break;

        case GeneReceive:
            other.energy -= 5;
            other.ChooseMate(this);
            break;

        case GeneSymmetric:
            energy -= 5;
            ChooseMate(&other);
            other.ChooseMate(this);
            break;
    }
}

void Creat::MutateBrain()
{
    while (RandBool(mprofile.mutation_prob))
    {
        Pos w = SelectRandomWeight();
        weights(w) += RandGauss(0, mprofile.mutation_sd);

        if (lineages) AddToLineage(w);
    }

    if (mprofile.color_drift)
        marker += RandFloat(-0.015, 0.015);
}

void Creat::SanityCheck()
{
    bool insane = false;
    for (int i = 0; i < neurons; i++)
    {
        if (isinf(state(i)) || isnan(state(i)))
        {
            cout << "Neuron " << i  << " is insane!" << endl;
            flush(cout);
            insane = true;
        }
        for (int j = 0; j < neurons; j++)
        {
            float w = weights.Get(i,j);
            if (isinf(w) || isnan(w) || fabs(w) > 1000)
            {
                cout << "Weight " << i << ", " << j << " is insane!" << endl;
                insane = true;
            }
        }
    }
    if  (insane)
    {
        PRINT(marker);
        PRINT(pos.row);
        PRINT(pos.col);
        PRINT(weights);
        PRINT(state);
        PRINT(state2);
    }
    assert(!insane);
}
  
void Creat::Step()
{
    assert(alive);
  
    // SETUP EXTERNAL INPUTS
    state(0) = grid->EnergyKernel(pos, orient) / 20.0;
    state(1) = grid->EnergyKernel(pos, orient - 1) / 20.0;
    state(2) = grid->EnergyKernel(pos, orient + 1) / 20.0;
    state(3) = grid->CreatKernel(pos, orient);
    state(4) = grid->CreatKernel(pos, orient - 1);
    state(5) = grid->CreatKernel(pos, orient + 1);

    // SETUP INTERNAL INPUTS
    state(extinputs + 0) = 1.0;
    state(extinputs + 1) = (energy - 50.) / 50.;
    state(extinputs + 2) = (float(age) - (maxage / 2)) / maxage;
    state(extinputs + 3) = RandFloat(-1.0, 1.0);

    // CALCULATE BRAIN STEP

    for  (int j = Creat::inputs; j < Creat::inputs + Creat::hidden; j++)
    {
        float v = 0;
        for (int k = 0; k < Creat::inputs + Creat::hidden; k++)
            v += weights.data[j * Creat::neurons + k] * state.data[k];
        state2.data[j] = v;
    }

    for (int i = 0; i < hidden; i++)
    {
        float& h = state2(inputs + i);
        h = atanh(h);
    }

    for  (int j = Creat::inputs + Creat::hidden; j < Creat::neurons; j++)
    {
        float v = 0;
        for (int k = 0; k < Creat::inputs + Creat::hidden; k++)
            v += weights.data[j * Creat::neurons + k] * state.data[k];
        state2.data[j] = v;
    }

    SwapContents(state, state2);

    // CALCULATE ACTION
    action = ActionNone;
    float maxaction = 0.8;
    float* output = &state(inputs + hidden);
    for (int i = 0; i < NumberActions-1; i++)
    {
        float f = output[i];
        if (f >= maxaction) {
            maxaction = f;
            action = i + 1;
        }
    }
    if (possessed) { action = ActionNone; possessed = false; }

    // CALCULATE ACTION COST
    energy -= actioncost[action];

    // UPDATE AGE
    age++;
    steps++;  
    special = false;

    // PERFORM ACTION
    if (energy > 0) (this->*(actionlookup[action]))();

    if (energy < 0 || age > maxage) Remove();
}

void Creat::__Remove()
{
    alive = false;
    grid->ncreats--;
    if (lineage) lineage->Decrement();
    if (desired_id >= 0) grid->creats[desired_id].desirer_id = -1;
    if (desirer_id >= 0) grid->creats[desirer_id].desired_id = -1;	
}

void Creat::TurnLeft()
{
    orient = Mod(orient - 1, 4);
}



void Creat::TurnRight()
{
    orient = Mod(orient + 1, 4);
}

void Creat::CopyBrain(Creat& parent)
{
    weights = parent.weights;
    lineage = parent.lineage; if (lineage) lineage->Increment();
    //  state = parent.state;

    marker = parent.marker;

    if (desired_id >= 0) BlendBrain(grid->creats[desired_id]);
}

void Creat::ChooseMate(Creat* other)
{
    if (desired_id >= 0) grid->creats[desired_id].desirer_id = -1;
    other->desirer_id = id;
    desired_id = other ? other->id : -1;
}

Pos Creat::Front(int offset)
{
    Pos front = pos + Pos(orient + offset);
    return front.Wrap(grid->rows, grid->cols);
}

Creat* Creat::Peer(int id)
{
    if (id < 0) return NULL;
    else return &grid->creats[id];
}

float Creat::Complexity()
{
    return weights.GetHammingNorm();
}

vector<Matrix> ReconstructBrains(list<LineageNode>& lineage, Matrix& initial, int T)
{
    vector<Matrix> brains;
    Matrix brain = initial;

    int next = T;
    list<LineageNode>::iterator it = lineage.begin();

    while (it != lineage.end())
    {
        if (T == 0) brains.push_back(brain);
        else
        { 
            while ((it->timestep+1) >= next)
            {
                next += T;
                brains.push_back(brain);
            }
        }
        int time = it->timestep;
        while (time == it->timestep && it != lineage.end())
        {
            brain(it->pos) = it->value;
            it++;
        } 
    }
    brains.push_back(brain);

    return brains;
}


void ShuffleBrains(Matrix& a, Matrix& b)
{
    for (int i = 0; i < Creat::neurons; i++)
        for (int j = 0; j < Creat::neurons; j++)
        {
            if (RandBit()) Swap(a(i,j), b(i,j));
        }
}

void Creat::BlendBrain(Creat& other)
{
    for (int i = 0; i < neurons; i++)
        for (int j = 0; j < neurons; j++)
            if (RandBit()) weights(i,j) = other.weights(i,j);
}
