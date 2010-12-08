#include "grid.hpp"
#include "creat.hpp"
#include <fstream>
#include <assert.h>

#include <QPainter>

RegisterClass(Grid, None)
RegisterVar(Grid, energy)
RegisterVar(Grid, energy_decay_rate)
RegisterVar(Grid, timestep)
RegisterVar(Grid, births)
RegisterVar(Grid, num_creats)
RegisterVar(Grid, interaction_type)
RegisterVar(Grid, enable_mutation)
RegisterVar(Grid, enable_respawn)
RegisterVar(Grid, path_energy)
RegisterVar(Grid, record_lineages)
RegisterVar(Grid, initial_energy)
RegisterVar(Grid, initial_marker)
//RegisterVar(Grid, action_cost)
//RegisterVar(Grid, initial_brain)
RegisterVar(Grid, max_age)
RegisterVar(Grid, total_steps)
RegisterVar(Grid, next_id)
RegisterVar(Grid, weight_mask)
RegisterVar(Grid, mutation_color_drift)
RegisterVar(Grid, mutation_prob)
RegisterVar(Grid, mutation_sd)

RegisterQtHook(Grid, mutation_color_drift, "Plumage Drift", QCheckBox);
RegisterQtHook(Grid, enable_mutation, "Mutation", QCheckBox);
RegisterQtHook(Grid, enable_respawn, "Respawning", QCheckBox);
RegisterQtHook(Grid, max_age, "Maximum age", QSpinBox);
RegisterQtHook(Grid, mutation_prob, "Mutation probability", ProbabilityBox);
RegisterQtHook(Grid, initial_energy, "Initial energy", QDoubleSpinBox);

void write_grid_size(Grid* g, std::ostream& s)
{
    s << "[" << g->rows << ", " << g->cols << "]";
}

void read_grid_size(Grid* g, std::istream& s)
{
    int rows, cols;
    s >> "[" >> rows >> ", " >> cols >> "]";
    g->SetSize(rows, cols);
}
RegisterCustomVar(Grid, size, write_grid_size, read_grid_size)

RegisterVar(Grid, occupant_list)

void write_grid_occupant_order(Grid* g, std::ostream& s)
{
    std::list<int> order;
    for (int i = 0; i < g->rows * g->cols; i++)
    {
        Occupant* occ = g->occupant_grid[i];
        while (occ)
        {
            order.push_back(occ->id);
            occ = occ->next;
        }
    }
    s << order;
}
void read_grid_occupant_order(Grid* g, std::istream& s)
{
    std::list<int> order;
    s >> order;
    for_iterate(it, order)
    {
        short id = *it;
        Occupant* occ = g->LookupOccupantByID(id);

        assert(occ);
        occ->Attach(*g, occ->pos);
        occ->id = id;
    }
}
RegisterCustomVar(Grid, occupant_order, write_grid_occupant_order, read_grid_occupant_order)

using namespace std;

const char* interactionnames[] = {"None", "Wastage","Parasitism", "Predation", "Co-operation", "Gene Swap", "Gene Give", "Gene Receive", "Gene Symmetric", NULL};

Grid::Grid() : weight_mask(Creat::neurons, Creat::neurons)
{
    initial_brain = NULL;
    occupant_grid = NULL;

    energy_decay_rate = 0.0;
    enable_mutation = true;
    enable_respawn = false;
    path_energy = 0;

    mutation_color_drift = true;
    mutation_prob = 0.0;
    mutation_sd = 3.0;

    max_age = 100;
    total_steps = 0;
    next_id = 0;

    initial_energy = 2;
    initial_marker = 0.0;
    record_lineages = false;

    accuracy = 10;
    timestep = 0;
    num_creats = 0;
    births = 0;
    interaction_type = NoInteraction;

    SetupMask(true);
    SetupActions();
}

Grid::~Grid()
{
    // for (unsigned int i = 0; i < watchers.size(); i++)
    //    watchers[i]->grid = NULL;
}

void Grid::SetupMask(bool hid)
{
    int inl =  0;
    int inr =  Creat::inputs - 1;
    int hidl = Creat::inputs;
    int hidr = Creat::inputs + Creat::hidden - 1;
    int outl = hidr + 1;
    int outr = hidr + Creat::outputs;

    weight_mask.SetZero();
    if (hid)
    {
        weight_mask.SetSubMatrix(hidl, inl, hidr, inr, 1.0);
        weight_mask.SetSubMatrix(outl, hidl, outr, hidr, 1.0);
        weight_mask.SetSubMatrix(hidl, hidl, hidr, hidr, 1.0);
    }
    weight_mask.SetSubMatrix(outl, inl, outr, inr, 1.0);
}

void Grid::SetSize(int rs, int cs)
{
    rows = rs;
    cols = cs;

    energy.Resize(rows, cols);

    if (occupant_grid) delete occupant_grid;

    occupant_grid = new Occupant*[rows * cols];

    for (int i = 0; i < rows * cols; i++)
        occupant_grid[i] = NULL;

    for_iterate(creat, graveyard)
        delete *creat;

    graveyard.clear();
    for_iterate(occ, occupant_list)
        delete *occ;

    occupant_list.clear();
}

void Grid::SetupActions()
{
    action_lookup[ActionNone] = &Creat::DoNothing;
    action_lookup[ActionForward] = &Creat::MoveForward;
    action_lookup[ActionLeft] = &Creat::TurnLeft;
    action_lookup[ActionRight] = &Creat::TurnRight;
    action_lookup[ActionReproduce] = &Creat::Reproduce;

    action_cost[ActionNone] = 0;
    action_cost[ActionLeft] = 0;
    action_cost[ActionForward] = 1.0;
    action_cost[ActionRight] = 0;
    action_cost[ActionReproduce] = 60.0;
}

Pos Grid::RandomCell()
{
    return Pos(RandInt(rows-1), RandInt(cols-1));
}

Pos Grid::EmptyCell()
{
    Pos pos = RandomCell();
    while (OccupantAt(pos)) pos = RandomCell();

    return pos;
}

Pos Grid::FairCell()
{
    float thresh = 10;
    for (int i = 0; i < 100; i++)
    {
        Pos pos = EmptyCell();
        if (energy(pos) > thresh) return pos;
        thresh -= 0.05;
    }
    return EmptyCell();
}

void Grid::AddCreats(int number, bool fairly)
{
    for (int i = 0; i < number; i++)
        AddCreatAt(fairly ? FairCell() : EmptyCell(), RandInt(3));
}

Creat& Grid::AddCreatAt(Pos pos, int orient)
{
    Creat& creat = _AddCreat(pos, orient);

    if (initial_brain) creat.weights = *initial_brain;

    return creat;
}

Creat& Grid::_AddCreat(Pos pos, int orient)
{
    num_creats++;  

    Creat* fresh = NULL;
    if (graveyard.size())
    {
        fresh = graveyard.back();
        graveyard.pop_back();
    } else {
        fresh = new Creat;
    }

    fresh->Attach(*this, pos);
    fresh->AssignID();
    fresh->Reset();
    fresh->orient = orient;
    fresh->alive = true;

    return *fresh;
}

void Grid::RemoveOccupants()
{
    while (occupant_list.size())
    {
        occupant_list.back()->Remove();
    }
}
  
Occupant* Grid::LookupOccupantByID(int search_id)
{
    for_iterate(it, occupant_list)
    {
        if ((*it)->id == search_id) return *it;
    }
    return NULL;
}

Creat* Grid::FindCreat(int marker)
{
    if (num_creats == 0) return NULL;
  
    Creat* found = NULL;
    float max = 0;
    for_iterate(it, occupant_list)
    {
        if (Creat* creat = dynamic_cast<Creat*>(*it))
        {
            if (creat->energy > max && (!marker || (creat->marker == marker)))
            {
                max = creat->energy;
                found = creat;
                break;
            }
        }
    }
    return found;
}

#define KERNEL(r2,c2) data[cs * Mod(r + r2, rs) + Mod(c + c2, cs)]
#define KERNEL2(x,y) Kernel2(occupant_grid[cs * Mod(r + x, rs) + Mod(c + y, cs)])
static inline float Kernel2(Occupant* occ)
{ return occ ? occ->signature : 0; }

float Grid::EnergyKernel(Pos pos, int dir)
{
    int r = pos.row, c = pos.col;
    int cs = cols, rs = rows;
    float* data = energy.data;
    switch(Mod(dir, 4))
    {
        case 3:
            return
                2 * KERNEL(0,-1) 
                + 0.5 * (KERNEL(0,-2) + KERNEL(-1,-1) + KERNEL(1,-1));
        case 2:
            return
                2 * KERNEL(1,0) 
                + 0.5 * (KERNEL(2,0) + KERNEL(1,-1) + KERNEL(1,1));
        case 1:
            return
                2 * KERNEL(0,1) 
                + 0.5 * (KERNEL(0,2) + KERNEL(-1,1) + KERNEL(1,1));
        case 0:
            return
                2 * KERNEL(-1,0) 
                + 0.5 * (KERNEL(-2,0) + KERNEL(-1,-1) + KERNEL(-1,1));
    }
    return 0;
}

float Grid::CreatKernel(Pos pos, int dir)
{
    int r = pos.row, c = pos.col;
    int cs = cols, rs = rows;
    switch(Mod(dir, 4))
    {
        case 3:
            return 
                2 * KERNEL2(0,-1) 
                + 0.5 * (KERNEL2(0,-2) + KERNEL2(-1,-1) + KERNEL2(1,-1));
        case 2:
            return
                2 * KERNEL2(1,0) 
                + 0.5 * (KERNEL2(2,0) + KERNEL2(1,-1) + KERNEL2(1,1));
        case 1:
            return
                2 * KERNEL2(0,1) 
                + 0.5 * (KERNEL2(0,2) + KERNEL2(-1,1) + KERNEL2(1,1));
        case 0:
            return
                2 * KERNEL2(-1,0) 
                + 0.5 * (KERNEL2(-2,0) + KERNEL2(-1,-1) + KERNEL2(-1,1));
    }
    return 0;
}

float aligned(Creat* creat, int d)
{
    if (creat == NULL) return 0;
    d = Mod(creat->orient - d, 4);
    if (d == 0) return 1;
    if (d == 2) return -1;
    return 0;
}
#define KERNEL3(x,y) aligned(occupant_grid[cs * Mod(r + x, rs) + Mod(c + y, cs)], creat.orient)

void Grid::Run(int steps, int report)
{
    for (int i = 0; i < steps; i++)
    {
        if (report && (timestep % report) == 0) Report();
        Step();
    }
}
/*
void Grid::RunLineage(const char* file, int steps, int every)
{
    record_lineages = true;
    timestep = 0;

    for (int i = 1; i <= steps; i++)
    {
        if (i % every == 0) Report();
        Step();
    }
    record_lineages = false;
    enable_mutation = false;
    Run(2000);
    enable_mutation = true;

    if (num_creats == 0)
    {
        cout << "Ended history early due to extinction." << endl;
        return;
    }

    Creat* good = FindCreat(0);
    list<LineageNode> lineage = good->ReconstructLineage();

    if (true) // 'equilineage' should be a parameter
    {
        vector<Matrix> history = ReconstructBrains(lineage, *initial_brain, every);
        WriteMatrices(file, history);
    } else {
        vector<Matrix> history2 = ReconstructBrains(lineage, *initial_brain, 0);
        vector<Matrix> history;
  
        int m = history2.size();
        int n = steps / every;
        for (int i = 0; i <= n - 1; i++)
        {
            history.push_back(history2[round( (m-1) * i/float(n - 1))]);
        }

        WriteMatrices(file, history);
    }
}*/

void Grid::Report()
{
    float avg_complexity = 0;

    for_iterate(it, occupant_list)
    {
        if (Creat* creat = dynamic_cast<Creat*>(*it))
        {
            avg_complexity += creat->Complexity();
        }
    }

    avg_complexity /= num_creats ? num_creats : 1;
    cout << "Time, Pop, Complexity = " << timestep << " " << num_creats << " " << avg_complexity << endl;

    flush(cout);
}

void Grid::Step()
{
    list<Occupant*>::iterator it = occupant_list.begin();
    while (it != occupant_list.end())
    {
        (*it++)->Update();
    }

    if (energy_decay_rate != 0.0) energy *= (1.0 - energy_decay_rate);

    if (num_creats == 0 && enable_respawn)
        AddCreats(50, true);

    timestep++;
}

float CompeteFunction(int numa, int numb)
{
    if (numa == 0 && numb == 0) return 0;
    return atan(log(float(numa) / float(numb))) / (3.14159265357979 / 2);
}

float Grid::CompeteScore(Matrix& a, Matrix& b)
{
    Matrix* last = initial_brain;

    enable_mutation = false;
  
    double scores = 0;
    for (int m = 0; m < accuracy; m++)
    {
        RemoveOccupants();

        for (int i = 0; i < 50; i++)
        {
            initial_brain = &a;
            initial_marker = 0.0;
            AddCreatAt(FairCell());
            initial_brain = &b;
            initial_marker = 1.0;
            AddCreatAt(FairCell());
        }

        // was 200
        for (int t = 0; t < 500; t++) Step(); //  XXX: Turn off watching here

        int numa = CountCreatsByMarker(0.0);
        int numb = CountCreatsByMarker(1.0);

        scores += CompeteFunction(numa, numb);
    }
    scores /= accuracy;
  
    initial_brain = last;
    enable_mutation = true;
  
    return scores;
}

int Grid::CountCreatsByMarker(int marker)
{
  int n = 0;
  for_iterate(it, occupant_list)
  {
      if (Creat* creat = dynamic_cast<Creat*>(*it))
      {
          if (creat->marker == marker) n++;
      }
  }
  return n;
}

void Grid::Reset()
{
    RemoveOccupants();
}

/*
void Grid::LoadOccupant(istream& is)
{
    int id;
    char name[128];

    is >> id;
    is >> name;

    Occupant* occ;
    if (id == -1)
        occ = dynamic_cast<Occupant*>(MetaClass::MakeNew(name));
    else
        occ = &creats[id];

    occ->grid = this;
    occ->Read(is);
    occ->Place();
}


void Grid::SaveOccupant(std::ostream& os, Occupant* occ)
{
    Creat* cr = dynamic_cast<Creat*>(occ);

    os << (cr ? cr->id : -1) << endl
       << occ->Name() << endl;

    occ->Write(os);
}
*/
Matrix Grid::FindDominantGenome()
{
    enable_mutation = false;
    Run(1000);
    Creat* c = FindCreat(0);
    enable_mutation = true;
    assert(c);
    // cout << "Dominant genome found." << endl;
    return c->weights;
}

Matrix Grid::Evolve(int steps)
{
    // cout << "Beginning evolution of " << steps << " steps." << endl;
    RemoveOccupants();
    AddCreats(50,true);
    Run(steps);//, steps/5);
    return FindDominantGenome();
}

void Grid::Paint(QImage& image)
{
    QRgb red = qRgb(255,0,0);
    for (int i = 0; i < rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(image.scanLine(i));
        float* line2 = energy[i];
        for (int j = 0; j < cols; j++)
        {
            int val = *line2++ * 5;
            if (OccupantAt(Pos(i,j)))
                *line1++ = red;
            else
                *line1++ = qRgb(val,val,val);
        }
    }
}

void Grid::Paint(QPainter& painter)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int value = energy(i,j) * 5;
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            painter.fillRect(j,i,1,1,QColor(value,value,value));
        }
    }
}
