#include "grid.hpp"
#include "creat.hpp"
#include "feeder.hpp"
#include <fstream>
#include  <assert.h>

#include <QPainter>

using namespace std;

const char* interactionnames[] = {"None", "Wastage","Parasitism", "Predation", "Co-operation", "Gene Swap", "Gene Give", "Gene Receive", "Gene Symmetric", NULL};

Grid::Grid(int rs, int cs)
    : adam(NULL),
      lastadam(NULL),
      feeder(NULL),
      energy(rs,cs),
      occupants(NULL)
{
    cols = cs;
    rows = rs;
    occupants = new Occupant*[rows * cols];
    for (int i = 0; i < rows * cols; i++) occupants[i] = NULL;

    equilineage = false;
    decay = 1.0;
    mutation = true;
    birth = true;
    respawn = false;
    pathenergy = 0;

    feeding = true;
    freespot = 0;
    accuracy = 10;
    timestep = 0;
    ncreats = 0;
    births = 0;
    interaction = NoInteraction;
    energy.SetZero();

    for (int i = 0; i < maxcreats; i++)
    {
        creats[i].id = i;
        creats[i].grid = this;
    }
}


void Grid::PushAdam(Matrix* a)
{
    lastadam = adam;
    adam = a;
}

void Grid::PopAdam()
{
    adam = lastadam;
}

Grid::~Grid()
{
    if (feeder) feeder->grid = NULL;
    // for (unsigned int i = 0; i < watchers.size(); i++)
    //    watchers[i]->grid = NULL;
}

void Grid::Resize(int r, int c)
{
    RemoveAllCreats();

    rows = r, cols = c;
    energy.Resize(rows, cols);
    delete[] occupants;
    occupants = new Occupant*[rows * cols];
    for (int i = 0; i < rows * cols; i++) occupants[i] = NULL;

    if (feeder) feeder->Resize();

    cout << "Resized to dimensions " << rows << ", " << cols << endl;
}

void Grid::AttachFeeder(Feeder& f)
{
    feeder = &f;
    feeder->SetGrid(this);
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

    if (adam) creat.weights = *adam;

    return creat;
}

Creat& Grid::_AddCreat(Pos pos, int orient)
{
    ncreats++;
  
    assert (ncreats <= maxcreats);

    int j = 0;
    // int j = freespot;
    // if (j < 0)
    // {
    //   j = 0;
    //   while (creats[j].alive) j++;

    // } else freespot = -1;
    while (creats[j].alive) j++;
    if (j >= maxcreats) assert(0);
  
    Creat& creat = creats[j];
    creat.orient = orient;
    creat.pos = pos;
    creat.Reset();
    creat.Place();
    creat.alive = true;

    return creat;
}

void Grid::RemoveAllCreats()
{
    for (int i = 0; i < maxcreats; i++) creats[i].Remove();
}
  
Creat* Grid::FindCreat(float marker)
{
    if (ncreats == 0) return NULL;
  
    Creat* c = NULL;
    float max = 0;
    for (int i = 0; i < maxcreats; i++)
    {
        if (creats[i].alive
            && (!marker || (creats[i].marker == marker))
            && creats[i].energy > max)
        {
            max = creats[i].energy;
            c = &creats[i];
            break;
        }
    }
    return c;
}

#define NEWKERNEL
#define KERNEL(x,y) data[cs * Mod(r + x, rs) + Mod(c + y, cs)]  
#define KERNEL2(x,y) Kernel2(occupants[cs * Mod(r + x, rs) + Mod(c + y, cs)])
static inline float Kernel2(Occupant* occ)
{ return occ ? occ->signature : 0; }

#ifdef NEWKERNEL
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
                + 0.5 * (KERNEL(0,-2) + KERNEL(-1,-1) + KERNEL(-1,-1));
        case 2:
            return
                2 * KERNEL(1,0) 
                + 0.5 * (KERNEL(2,0) + KERNEL(1,-1) + KERNEL(1,1));
        case 1:
            return
                2 * KERNEL(0,1) 
                + 0.5 * (KERNEL(0,2) + KERNEL(-1,1) + KERNEL(-1,1));
        case 0:
            return
                2 * KERNEL(-1,0) 
                + 0.5 * (KERNEL(-2,0) + KERNEL(-1,-1) + KERNEL(-1,1));
    }
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
                + 0.5 * (KERNEL2(0,-2) + KERNEL2(-1,-1) + KERNEL2(-1,-1));
        case 2:
            return
                2 * KERNEL2(1,0) 
                + 0.5 * (KERNEL2(2,0) + KERNEL2(1,-1) + KERNEL2(1,1));
        case 1:
            return
                2 * KERNEL2(0,1) 
                + 0.5 * (KERNEL2(0,2) + KERNEL2(-1,1) + KERNEL2(-1,1));
        case 0:
            return
                2 * KERNEL2(-1,0) 
                + 0.5 * (KERNEL2(-2,0) + KERNEL2(-1,-1) + KERNEL2(-1,1));
    }
}
#else
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
                + 0.5 * (KERNEL(0,-2) + KERNEL(-1,-2) + KERNEL(1,-2) + KERNEL(-1,-1) + KERNEL(-1,-1));
        case 2:
            return
                2 * KERNEL(1,0) 
                + 0.5 * (KERNEL(2,0) + KERNEL(2,-1) + KERNEL(2,1) + KERNEL(1,-1) + KERNEL(1,1));
        case 1:
            return
                2 * KERNEL(0,1) 
                + 0.5 * (KERNEL(0,2) + KERNEL(-1,2) + KERNEL(1,2) + KERNEL(-1,1) + KERNEL(-1,1));
        case 0:
            return
                2 * KERNEL(-1,0) 
                + 0.5 * (KERNEL(-2,0) + KERNEL(-2,-1) + KERNEL(-2,1) + KERNEL(-1,-1) + KERNEL(-1,1));
    }
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
                + 0.5 * (KERNEL2(0,-2) + KERNEL2(-1,-2) + KERNEL2(1,-2) + KERNEL2(-1,-1) + KERNEL2(-1,-1));
        case 2:
            return
                2 * KERNEL2(1,0) 
                + 0.5 * (KERNEL2(2,0) + KERNEL2(2,-1) + KERNEL2(2,1) + KERNEL2(1,-1) + KERNEL2(1,1));
        case 1:
            return
                2 * KERNEL2(0,1) 
                + 0.5 * (KERNEL2(0,2) + KERNEL2(-1,2) + KERNEL2(1,2) + KERNEL2(-1,1) + KERNEL2(-1,1));
        case 0:
            return
                2 * KERNEL2(-1,0) 
                + 0.5 * (KERNEL2(-2,0) + KERNEL2(-2,-1) + KERNEL2(-2,1) + KERNEL2(-1,-1) + KERNEL2(-1,1));
    }
}
#endif


float aligned(Creat* creat, int d)
{
    if (creat == NULL) return 0;
    d = Mod(creat->orient - d, 4);
    if (d == 0) return 1;
    if (d == 2) return -1;
    return 0;
}
#define KERNEL3(x,y) aligned(occupants[cs * Mod(r + x, rs) + Mod(c + y, cs)], creat.orient)

void Grid::Run(int steps, int report)
{
    timestep = 0;
    for (int i = 0; i < steps && ncreats; i++)
    {
        if (report && i % report == 0) Report();
        Step();
    }
}

void Grid::RunHistory(const char* file, int steps, int every)
{
    ofstream of;
    of.open(file);
    of.close();
   
    for (int i = 1; i <= steps && ncreats; i++)
    {
        if (i % every == 0)
        {
            Report();
            Matrix m = FindDominantGenome();
            WriteMatrix(file, m, true);
        }
        Step();
    }
    if (ncreats == 0)
    {
        cout << "Ended history early due to extinction." << endl;
    }
}  

void Grid::RunLineage(const char* file, int steps, int every)
{
    Creat::lineages = true;
    timestep = 0;

    for (int i = 1; i <= steps; i++)
    {
        if (i % every == 0) Report();
        Step();
    }
    Creat::lineages = false;
    mutation = false;
    Run(2000);
    mutation = true;

    if (ncreats == 0)
    {
        cout << "Ended history early due to extinction." << endl;
        return;
    }

    Creat* good = FindCreat(0);
    list<LineageNode> lineage = good->ReconstructLineage();

    if (!equilineage)
    {
        vector<Matrix> history = ReconstructBrains(lineage, *adam, every);
        WriteMatrices(file, history);
    } else {
        vector<Matrix> history2 = ReconstructBrains(lineage, *adam, 0);
        vector<Matrix> history;
  
        int m = history2.size();
        int n = steps / every;
        for (int i = 0; i <= n - 1; i++)
        {
            history.push_back(history2[round( (m-1) * i/float(n - 1))]);
        }

        WriteMatrices(file, history);
    }
}

void Grid::Report()
{
    cout << "Time, Pop      = \t" << timestep << "\t" << ncreats << endl;
    flush(cout);
}

void Grid::Step()
{
    for (int i = 0; i < maxcreats; i++)
        if (creats[i].alive) creats[i].Step();
  
    for (int i = 0; i < rows * cols; i++)
        if (occupants[i]) occupants[i]->Update();  

    if (feeder and feeding) feeder->Feed();

    if (decay != 1.0) energy *= decay;

    timestep++;
}

float CompeteFunction(int numa, int numb)
{
    if (numa == 0 && numb == 0) return 0;
    return atan(log(float(numa) / float(numb))) / (3.14159265357979 / 2);
}

float Grid::CompeteScore(Matrix& a, Matrix& b)
{
    PushAdam();
    mutation = false;
  
    double scores = 0;
    for (int m = 0; m < accuracy; m++)
    {
        RemoveAllCreats();
        feeder->InitialFeed();

        for (int i = 0; i < 50; i++)
        {
            adam = &a;
            Creat::initialmarker = 0.0;
            AddCreatAt(FairCell());
            adam = &b;
            Creat::initialmarker = 1.0;
            AddCreatAt(FairCell());
        }

        // was 200
        for (int t = 0; t < 500; t++) Step(); //  XXX: Turn off watching here

        int numa = CountCreatsByMarker(0.0);
        int numb = CountCreatsByMarker(1.0);

        scores += CompeteFunction(numa, numb);
    }
    scores /= accuracy;
  
    PopAdam();
    mutation = true;
  
    return scores;
}

int Grid::CountCreatsByMarker(float marker)
{
  int n = 0;
  for (int i = 0; i < maxcreats; i++)
  {
    if (creats[i].alive && creats[i].marker == marker) n++;
  }
  return n;
}

void Grid::SaveState(ostream& os)
{
    assert (interaction <= Cooperation);
    SanityCheck();

    os << rows << " "
       << cols << endl
       << energy << endl

       << randomstate << endl
  
       << freespot << endl
       << decay << endl
       << timestep << endl
       << ncreats << endl
       << interaction << endl
       << mutation << endl
       << birth << endl
       << respawn << endl
       << feeding << endl
       << accuracy << endl;

    int noccupants = 0;
    for (int i = 0; i < rows * cols; i++) 
        if (occupants[i]) noccupants++;
    os << noccupants << endl;
    for (int i = 0; i < rows * cols; i++) 
        if (occupants[i]) SaveOccupant(os, occupants[i]);
  
  
    if (feeder) feeder->SaveState(os);

    cout << "Grid state saved." << endl;
}

void Grid::LoadState(istream& is)
{
    int rs, cs;
    is >> rs >> cs;
    if (rs != rows || cs != cols)
    {
        cout << "Resizing to size " << rs << ", " << cs << endl;
        Resize(rs, cs);
    }

    RemoveAllCreats();
  
    is >> energy
       >> randomstate
       >> freespot
       >> decay
       >> timestep
       >> ncreats
       >> interaction
       >> mutation
       >> birth
       >> respawn
       >> feeding
       >> accuracy;

    int noccupants;
    is >> noccupants;
    while (noccupants--) LoadOccupant(is);

    SanityCheck();
  
    if (feeder) feeder->LoadState(is);

    cout << "Grid state loaded." << endl;
}

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

void Grid::SanityCheck()
{
    // ofstream os("sanity");
    // SaveState(os);
    // os.close();
    for (int i = 0; i < rows * cols; i++)
    {
        if (Creat* c = dynamic_cast<Creat*>(occupants[i]))
        {
            assert(c->alive);
        }
    }
    for (int i = 0; i < maxcreats; i++)
    {
        if (creats[i].alive)
        {
            assert(creats[i].pos.Inside(rows, cols));
            assert(OccupantAt(creats[i].pos) == &creats[i]);
            creats[i].SanityCheck();
        }
    }
}

Matrix Grid::FindDominantGenome()
{
    mutation = false;  
    Run(1000);
    Creat* c = FindCreat(0);
    mutation = true;
    assert(c);
    // cout << "Dominant genome found." << endl;
    return c->weights;
}

Matrix Grid::Evolve(int steps)
{
    // cout << "Beginning evolution of " << steps << " steps." << endl;
    RemoveAllCreats();
    feeder->InitialFeed();
    AddCreats(50,true);
    Run(steps);//, steps/5);
    return FindDominantGenome();
}

void Grid::Paint(QImage& image)
{
    Occupant* occ = *occupants;
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
            int value = energy.Get(i,j) * 5;
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            painter.fillRect(j,i,1,1,QColor(value,value,value));
        }
    }
}
