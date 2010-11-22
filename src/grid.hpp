#ifndef GRID_HPP
#define GRID_HPP

#include "misc.hpp"
#include "creat.hpp"

#include <iostream>
#include <vector>

class Creat;
class Feeder;

enum InteractionMode
{
    NoInteraction,
    Wastage,
    Parasitism,
    Predation,
    Cooperation,
    GeneExchange,
    GeneGive,
    GeneReceive,
    GeneSymmetric,
};

class Occupant;
class QPainter;
class QImage;

class Grid
{
public:      
    
    static const int maxcreats = 500;
    Creat creats[maxcreats];
    int freespot;

    Feeder* feeder;
    Matrix energy;
    float decay;

    int timestep;
    int births;
    int rows, cols;
    int ncreats;
    int interaction;

    bool equilineage;
    bool mutation;
    bool birth;
    bool respawn;
    bool feeding;
    int pathenergy;

    int accuracy;

    Matrix* adam;    
    Occupant** occupants;

    // Paint
    void Paint(QPainter& painter);
    void Paint(QImage& image);

    // State change
    Grid(int rws, int cls);
    ~Grid();
    void Resize(int rws, int cls);
    void SaveState(std::ostream& os);
    void LoadState(std::istream& is);
    void SaveOccupant(std::ostream& os, Occupant* occ);
    void LoadOccupant(std::istream& is);

    // External class stuff
    void AttachFeeder(Feeder& f);

    // Low-level position
    Pos Wrap(Pos pos) { return pos.Wrap(rows, cols); }
    Pos RandomCell();
    Pos EmptyCell();
    Pos FairCell();

    // Creat Management
    Creat& _AddCreat(Pos pos, int orient=0);
    Creat& AddCreatAt(Pos pos, int orient=0);
    void AddCreats(int number, bool fairly);
    void RemoveAllCreats();
    Creat* FindCreat(float marker);
    Occupant*& OccupantAt(Pos pos)
    { return occupants[pos.row * cols + pos.col]; }
    Creat* CreatAt(Pos pos)
    { return dynamic_cast<Creat*>(occupants[pos.row * cols + pos.col]); }

    // Scoring
    Matrix FindDominantGenome();
    float CompeteScore(Matrix& a, Matrix& b);
    int CountCreatsByMarker(float markers);

    void Step();

    // Convenience functions
    void Run(int steps, int report=0);
    void RunHistory(const char* file, int steps, int every);
    void RunLineage(const char* file, int steps, int every);
    void Report();
    Matrix Evolve(int steps);

    // Bug-checking
    void SanityCheck();

    // Creature vision functions
    float CreatKernel(Pos pos, int dir);
    float EnergyKernel(Pos pos, int dir);
};

float CompeteFunction(int numa, int numb);

#endif
