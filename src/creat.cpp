#include "creat.hpp"
#include "grid.hpp"
#include "qthookdefs.hpp"

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
RegisterVar(Creat, energy);
RegisterVar(Creat, marker);

RegisterQtHook(Creat, energy, "Energy", FloatHook(0,100,1));
RegisterQtHook(Creat, age, "Age", IntegerHook(0,1000));
RegisterQtHook(Creat, interaction_count, "Interactions", IntegerLabel());
RegisterQtHook(Creat, action, "Action", EnumHook("None\nForward\nLeft\nRight\nReproduce"));
RegisterQtHook(Creat, state, "Neurons", MatrixHook(7, true));
RegisterQtHook(Creat, weights, "Weights", MatrixHook(7, false));

/*
RegisterQtHook(Creat, orient, "Orientation", QSpinBox);
RegisterQtHook(Creat, action, "Action", QSpinBox);
*/
LineageNode::LineageNode(LineageNode* p)
    : pos(0,0), refs(1),  prev(p)
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
}

Creat::Creat()
    : weights(hidden + outputs, neurons),
      state(neurons, 1),
      state2(neurons, 1)
{
    Reset();
    signature = 1;
    weights.SetZero();
    state.SetZero();
    state2.SetZero();
    alive = false;
    pos.row = pos.col = 0;
}

Creat::Creat(const Creat &c) : Occupant()
{
    cerr << "creat being copied from creat with id: " << c.id << endl;
    throw "error";
}

void Creat::Reset()
{
    Occupant::Reset();
    state.SetZero();
    age = 0;
    fingerprint = 0;
    interacted = false;
    interaction_count = 0;
    for (int k = 0; k < 64; k++) { fingerprint <<= 1; fingerprint |= RandBit(); }
    action = ActionNone;
    possessed = false;
    alive = false;
    marker = grid ? grid->initial_marker : 0;
    energy = grid ? grid->initial_energy : 0;
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

Pos Creat::SelectRandomWeight()
{
    int j, k;
    j = RandInt(Creat::hidden + Creat::outputs - 1);
    k = RandInt(Creat::neurons-1);
    return Pos(j,k);
}

void Creat::Reproduce()
{
    Pos front = Front();

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
        interaction_count++;
        interacted = true;
    } else 
    {
        interacted = false;
        Move(front);
        float de = grid->energy(front);
        energy += de;
        grid->energy(pos) = grid->path_energy / 2.0;
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
    switch (grid->interaction_type)
    {
        case NoInteraction:
            break;

        case Penalty:
            energy -= 30;
            break;

        case Attack:
            energy -= 10;
            other.energy -= 50;
            break;

        case Zombie:
            energy -= 80;
            if (energy > 0) other.weights = weights;
            break;
      
        case Parasitism:
            other.TransferEnergy(*this, RandFloat(30,60));
            break;

        case Predation:
            other.TransferEnergy(*this, 100);
            break;

        case MutualCooperation:
            energy += 30;
            other.energy += 30;
            break;

        case AltruisticCooperation:
            energy -= 5;
            other.energy += 30;
            break;

        case GeneExchange: 
            energy -= 10;
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
    if (!grid->enable_mutation) return;

    int count = 0;
    bool mutated = false;
    while (RandBool(grid->mutation_prob) && count++ < 10)
    {
        mutated = true;

        Pos w = SelectRandomWeight();
        weights(w) += RandGauss(0, grid->mutation_sd);

        if (grid->record_lineages) AddToLineage(w);
    }

    if (mutated)
    {
        if (grid->mutation_color_drift)
            marker += RandSign() * 0.02;

        fingerprint <<= 1;
        fingerprint |= RandBit();
    }
}

void Creat::Update()
{
    Step();
}

// TODO: Update for new weight matrix
void Creat::CheckSanity(const char* str)
{
    for (int i = 0; i < Creat::neurons; i++)
    {
        float val = state(i);
        if (val != val)
        {
            cout << "Neuron " << i << " of creat " << id << " is " << val << endl;
            goto error;
        }
    }

    for (int i = 0; i < Creat::neurons; i++)
    {
        for (int j = 0; j < Creat::neurons; j++)
        {
            float val = weights(i,j);
            if (val != val)
            {
                cout << "ERROR: Weight " << i << ", " << j << " of creat " << id << " is " << val << endl;
                goto error;
            }
        }
    }

    if (pos.row < 0 || pos.row >= grid->rows ||
        pos.col < 0 || pos.col >= grid->cols)
    {
        cout << "ERROR: Position " << pos << " of creat " << id << " is invalid." << endl;
        goto error;
    }

    if (!alive)
    {
        cout << "ERROR: Creat " << id << " is dead!" << endl;
        goto error;
    }

    if (id == -1)
    {
        cout << "ERROR: Creat has invalid id: " << id << endl;
        goto error;
    }

    if (grid->LookupOccupantByID(id) != this)
    {
        cout << "ERROR: Wrong occupant found with id: " << id << endl;
        goto error;
    }

    {
        Occupant* occ = grid->OccupantAt(pos);
        while (occ)
        {
            if (occ == this) goto found;
            occ = occ->next;
        }
        cout << "ERROR: Occupant not found at position: " << pos << endl;
        goto error;
    }

    found:
    return;

    error:
    cout << "CONTEXT: " << str << endl;
    cout << "Creat printout:" << endl;
    cout << "State2 contents:" << state2 << endl;
    cout << "Creat::inputs = " << Creat::inputs << endl;
    cout << "Creat::inputs + Creat::hidden = " << Creat::inputs + Creat::hidden << endl;
    cout << "Creat::neurons = " << Creat::neurons << endl;

    cout << *this << endl;
    throw "invalid creat";
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
    state(extinputs + 2) = (float(age) - (grid->max_age / 2)) / grid->max_age;
    state(extinputs + 3) = RandFloat(-1.0, 1.0);

    for (int iter = 0; iter < grid->neural_net_iterations; iter++)
    {
        // CALCULATE BRAIN STEP
        // only clumns 0 through input+hidden are used
        // only rows from inputs from 0 through neu
        for  (int j = 0; j < Creat::hidden; j++)
        {
            float v = 0;
            for (int k = 0; k < Creat::inputs + Creat::hidden; k++)
                v += weights.data[j * Creat::neurons + k] * state.data[k];
            state2.data[Creat::inputs + j] = v;
        }

        for (int i = 0; i < hidden; i++)
        {
            float& h = state2(inputs + i);
            h = tanh(h);
        }

        for  (int j = Creat::hidden; j < Creat::hidden + Creat::outputs; j++)
        {
            float v = 0;
            for (int k = 0; k < Creat::inputs + Creat::hidden; k++)
                v += weights.data[j * Creat::neurons + k] * state.data[k];
            state2.data[Creat::inputs + j] = v;
        }
        SwapContents(state, state2);
    }
    //CheckSanity("After update");

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
    energy -= grid->action_cost[action];

    // UPDATE AGE
    age++;
    grid->total_steps++;

    // PERFORM ACTION
    if (energy > 0) (this->*(grid->action_lookup[action]))();

    if (energy < 0 || age > grid->max_age) Remove();
}

void Creat::__Remove()
{
    alive = false;
    grid->num_creats--;
    grid->graveyard.push_front(this);
    if (lineage) lineage->Decrement();
    if (desired_id >= 0) Peer(desired_id)->desirer_id = -1;
    if (desirer_id >= 0) Peer(desirer_id)->desired_id = -1;
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
    fingerprint = parent.fingerprint;

    if (desired_id >= 0) BlendBrain(*Peer(desired_id));
}

void Creat::ChooseMate(Creat* other)
{
    if (desired_id >= 0) Peer(desired_id)->desirer_id = -1;
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
    return (id < 0) ? NULL : dynamic_cast<Creat*>(grid->LookupOccupantByID(id));
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
    for (int i = 0; i < hidden + outputs; i++)
        for (int j = 0; j < neurons; j++)
            if (RandBit()) weights(i,j) = other.weights(i,j);
}
