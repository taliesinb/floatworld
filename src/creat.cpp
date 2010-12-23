#include "creat.hpp"

#include <fstream>

using namespace std;

RegisterClass(Creat, Occupant);
RegisterVar(Creat, desired_id);
RegisterVar(Creat, desirer_id);
RegisterVar(Creat, weights);
RegisterVar(Creat, state);
RegisterVar(Creat, action);
RegisterVar(Creat, age);
RegisterVar(Creat, last_orient);
RegisterVar(Creat, orient);
RegisterVar(Creat, alive);
RegisterVar(Creat, energy);
RegisterVar(Creat, marker);
RegisterVar(Creat, children);

const char* neuron_labels = "energy in front\n"
"energy on left\n"
"energy on right\n"
"creats in front\n"
"creats on left\n"
"creats on right\n"
"parallel orientation\n"
"perpendicular orientation\n"
"constant input\n"
"own energy\n"
"own age\n"
"random input\n"
"hidden 1\n"
"hidden 2\n"
"hidden 3\n"
"hidden 4\n"
"move forward\n"
"turn left\n"
"turn right\n"
"reproduce";

const char* outputs = "hidden 1\n"
                      "hidden 2\n"
                      "hidden 3\n"
                      "hidden 4\n"
                      "move forward\n"
                      "turn left\n"
                      "turn right\n"
                      "reproduce";

RegisterBinding(Creat, energy, "energy", 0, 500, 1);
RegisterBinding(Creat, age, "age", 0, 1000);
RegisterBinding(Creat, interaction_count, "interacts");
RegisterBinding(Creat, children, "children");
RegisterBinding(Creat, action, "action", "None\nForward\nLeft\nRight\nReproduce");
RegisterBinding(Creat, state, "neurons", 7, true, neuron_labels, "");
RegisterBinding(Creat, weights, "weights", 7, false, outputs, neuron_labels);

/*
RegisterBinding(Creat, orient, "Orientation", QSpinBox);
RegisterBinding(Creat, action, "Action", QSpinBox);
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
    : weights(num_hidden + num_outputs, num_neurons),
      state(num_neurons, 1),
      state2(num_neurons, 1)
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
    orient = 0;
    children = 0;
    last_orient = 0;
    interacted = false;
    interaction_count = 0;
    fingerprint = 0;
    for (int k = 0; k < 64; k++) { fingerprint <<= 1; fingerprint |= RandBit(); }
    action = ActionNone;
    alive = false;
    marker = world ? world->initial_marker : 0;
    energy = world ? world->initial_energy : 0;
    lineage = NULL;
    desired_id = -1;
    desirer_id = -1;
}

void Creat::Die()
{
    alive = false;
}

void Creat::HookWasChanged()
{
    UpdateBrain();
    UpdateQtHook();
}

void Creat::AddToLineage(Pos w)
{
    LineageNode* node = new LineageNode(lineage);
    node->timestep = world->timestep;
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
    j = RandInt(Creat::num_hidden + Creat::num_outputs - 1);
    k = RandInt(Creat::num_neurons-1);
    return Pos(j,k);
}

void Creat::Reproduce()
{
    Pos front = Front();

    if (world->OccupantAt(front))
        return;
  
    Creat& child = world->_AddCreat(front, Mod(orient,4));
    child.CopyBrain(*this);
    child.MutateBrain();

    float excess = state(num_inputs + num_hidden + ActionReproduce - 1) - 1.0;
    if (excess > 0) TransferEnergy(child, excess * 50);

    children++;
    world->births++;
}

void Creat::MoveForward()
{ 
    Pos front;
    if (int jump = world->jump_range)
    {
        float excess = ClipFloat((state(num_inputs + num_hidden + ActionForward - 1) - 1.2) * 2 * jump, 0, jump);
        int dist = round(excess);
        energy -= dist;
        front = world->Wrap(pos + Pos(orient) * (1 + dist));
    } else {
        front = Front();
    }

    if (Occupant* other = world->SolidOccupantAt(front))
    {
        interacted = true;
        other->Interact(*this);
        interaction_count++;
    } else
    {
        float de = world->energy(pos);
        energy += de;
        world->energy(pos) = world->path_energy;
        Move(front);
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
    switch (world->interaction_type)
    {
        case NoInteraction:
            break;

        case Penalty:
            energy -= 30;
            break;

        case Attack:
            energy -= 3;
            other.energy = -100;
            break;

        case Zombie:
            energy -= 50;
            if (energy > 0) other.weights = weights;
            break;
      
        case Parasitism:
            other.TransferEnergy(*this, RandFloat(30,60));
            break;

        case Predation:
            other.TransferEnergy(*this, 200);
            break;

        case Shoving: {
            Pos pos2 = world->Wrap(other.pos + Pos(orient));
            Creat* third = world->CreatAt(pos2);
            if (third)
            {
                int _orient = other.orient;
                other.orient = orient;
                other.Interaction(*third);
                other.orient = _orient;
            }
            if (!world->OccupantAt(pos2))
            {
                other.Move(pos2);
            }
        } break;

    case CongaLine: {
            Pos new_pos = other.Front();
            int new_orient = orient;
            int limit = 0;
            Creat* leader = world->CreatAt(new_pos);
            while (leader && limit++ < 20)
            {
                new_orient = leader->orient;
                new_pos = leader->Front();
                leader = world->CreatAt(new_pos);
            }
            if (!world->OccupantAt(new_pos) && limit < 20)
            {
                orient = new_orient;
                Move(new_pos);
            }

        } break;

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
    if (!world->enable_mutation) return;

    int count = 0;
    bool mutated = false;
    while (RandBool(world->mutation_prob) && count++ < 10)
    {
        mutated = true;

        Pos w = SelectRandomWeight();
        weights(w) += RandGauss(0, world->mutation_sd);

        if (world->record_lineages) AddToLineage(w);
    }

    if (mutated)
    {
        if (world->mutation_color_drift)
            marker += RandSign() * RandGauss(0.05, 0.03);

        fingerprint <<= 1;
        fingerprint |= RandBit();
    }
}

void Creat::UpdateBrain()
{
    for (int k = off_ext_inputs; k < off_ext_inputs + num_inputs; k++)
        state2(k) = state(k);

    for (int iter = 0; iter < world->neural_net_iterations; iter++)
    {
        // update the hiddens from the inputs and the old hiddens
        for  (int j = 0; j < num_hidden; j++)
        {
            float v = 0;
            for (int k = 0; k < num_inputs + num_hidden; k++)
                v += weights.data[j * num_neurons + k] * state.data[k];
            state2.data[off_hidden + j] = tanh(v);
        }

        // update the outputs from everyone
        for  (int j = 0; j < Creat::num_outputs; j++)
        {
            float v = 0;
            for (int k = 0; k < off_outputs; k++)
                v += weights.data[(j + num_hidden) * num_neurons + k] * state.data[k];
            state2.data[off_outputs + j] = v;
        }
        SwapContents(state, state2);
    }

    // CALCULATE ACTION
    action = ActionNone;
    float maxaction = 1.0;
    float* output = &state(off_outputs);
    for (int i = 0; i < NumberActions-1; i++)
    {
        float f = output[i];
        if (f >= maxaction) {
            maxaction = f;
            action = i + 1;
        }
    }
}

void Creat::UpdateInputs()
{
    // SETUP EXTERNAL INPUTS
    state(off_ext_inputs + 0) = world->EnergyAt(Front( 0, 2));
    state(off_ext_inputs + 1) = world->EnergyAt(Front(-1, 2));
    state(off_ext_inputs + 2) = world->EnergyAt(Front(+1, 2));
    state(off_ext_inputs + 3) = world->ColorAt(Front( 0,  1));
    state(off_ext_inputs + 4) = world->ColorAt(Front(-1,  1));
    state(off_ext_inputs + 5) = world->ColorAt(Front(+1,  1));
    state(off_ext_inputs + 6) = world->DirKernel(pos, orient);
    state(off_ext_inputs + 7) = world->DirKernel(pos, orient + 1);

    // SETUP INTERNAL INPUTS
    state(off_int_inputs + 0) = 1.0;
    state(off_int_inputs + 1) = (2.0 * energy / world->action_cost[ActionReproduce]) - 1.0;
    state(off_int_inputs + 2) = (2.0 * age / world->max_age) - 1.0;
    state(off_int_inputs + 3) = RandFloat(-1.0, 1.0);
}

// TODO: Update for new weight matrix
void Creat::CheckSanity(const char* str)
{
    for (int i = 0; i < Creat::num_neurons; i++)
    {
        float val = state(i);
        if (val != val)
        {
            cout << "Neuron " << i << " of creat " << id << " is " << val << endl;
            goto error;
        }
    }

    for (int i = 0; i < Creat::num_neurons; i++)
    {
        for (int j = 0; j < Creat::num_neurons; j++)
        {
            float val = weights(i,j);
            if (val != val)
            {
                cout << "ERROR: Weight " << i << ", " << j << " of creat " << id << " is " << val << endl;
                goto error;
            }
        }
    }

    if (pos.row < 0 || pos.row >= world->rows ||
        pos.col < 0 || pos.col >= world->cols)
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

    if (world->LookupOccupantByID(id) != this)
    {
        cout << "ERROR: Wrong occupant found with id: " << id << endl;
        goto error;
    }

    {
        Occupant* occ = world->OccupantAt(pos);
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
    cout << "Creat::inputs = " << Creat::num_inputs << endl;
    cout << "Creat::inputs + Creat::hidden = " << Creat::num_inputs + Creat::num_hidden << endl;
    cout << "Creat::neurons = " << Creat::num_neurons << endl;

    cout << *this << endl;
    throw "invalid creat";
}

void Creat::Update()
{
    if (!alive) { Remove(); return; }

    last_orient = orient;

    UpdateInputs();
    UpdateBrain();

    // CALCULATE ACTION COST
    energy -= world->action_cost[action];

    // UPDATE AGE
    age++;
    world->total_steps++;

    // PERFORM ACTION
    interacted = false;
    if (energy > 0) (this->*(world->action_lookup[action]))();

    if (energy < 0 || age > world->max_age) Die();
}

void Creat::__Remove()
{
    alive = false;
    world->num_creats--;
    world->graveyard.push_front(this);
    if (lineage) lineage->Decrement();
    if (desired_id >= 0) if (Creat* peer = Peer(desired_id)) peer->desirer_id = -1;
    if (desirer_id >= 0) if (Creat* peer = Peer(desirer_id)) peer->desired_id = -1;
}

void Creat::TurnLeft()
{
    last_orient = orient;
    orient = Mod(orient - 1, 4);
}

void Creat::TurnRight()
{
    last_orient = orient;
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
    if (desired_id >= 0) if (Creat* peer = Peer(desired_id)) peer->desirer_id = -1;
    if (other) other->desirer_id = id;
    desired_id = other ? other->id : -1;
}

Pos Creat::Front(int offset, int dist)
{
    return world->Wrap(pos + Pos(orient + offset) * dist);
}

Creat* Creat::Peer(int id)
{
    return (id < 0) ? NULL : dynamic_cast<Creat*>(world->LookupOccupantByID(id));
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
    for (int i = 0; i < a.Len(); i++)
        if (RandBit()) Swap(a.data[i], b.data[i]);
}

void Creat::BlendBrain(Creat& other)
{
    for (int i = 0; i < weights.Len(); i++)
        if (RandBit()) weights.data[i] = other.weights.data[i];
}
