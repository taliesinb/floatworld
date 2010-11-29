#ifndef GRID_HPP
#define GRID_HPP

#include "misc.hpp"
#include "creat.hpp"

#include <iostream>
#include <vector>

class Creat;
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
    
    static const int max_creats = 3000;
    Creat creats[max_creats];
    int freespot;

    Matrix energy;
    float energy_decay_rate;

    int timestep;
    int births;
    int rows, cols;
    int num_creats;
    int interaction_type;

    bool enable_mutation;
    bool enable_respawn;

    int path_energy;
    bool record_lineages;

    float action_cost[NumberActions];
    CreatFunc action_lookup[NumberActions];
    float initial_energy;
    float initial_marker;
    Matrix* initial_brain;
    int max_age;
    int total_steps;

    Matrix weight_mask;

    int accuracy;

    bool mutation_color_drift;
    float mutation_prob;
    float mutation_sd;

    Occupant** occupants;

    // Paint
    void Paint(QPainter& painter);
    void Paint(QImage& image);

    // State change
    Grid(int rws, int cls);
    ~Grid();

    // Setup
    void SetupMask(bool);
    void SetupActions();

    void Resize(int rws, int cls);
    void SaveState(std::ostream& os);
    void LoadState(std::istream& is);
    void SaveOccupant(std::ostream& os, Occupant* occ);
    void LoadOccupant(std::istream& is);

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

    // Creature vision functions
    float CreatKernel(Pos pos, int dir);
    float EnergyKernel(Pos pos, int dir);
};

float CompeteFunction(int numa, int numb);

#endif
