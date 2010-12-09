#ifndef GRID_HPP
#define GRID_HPP

#include "misc.hpp"
#include "creat.hpp"

#include <iostream>
#include <list>

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

enum {
    DrawAction,
    DrawAge,
    DrawEnergy,
    DrawColor
};

class Occupant;
class QPainter;
class QImage;

class Grid : public Object
{
public:      
    
    std::list<Occupant*> occupant_list;
    std::list<Creat*> graveyard;

    Matrix energy;
    float energy_decay_rate;

    int timestep;
    int births;
    int rows, cols;
    int num_creats;
    int interaction_type;

    int draw_type;
    bool draw_creats_only;

    bool enable_mutation;
    bool enable_respawn;

    int path_energy;
    bool record_lineages;

    float action_cost[NumberActions];
    CreatFunc action_lookup[NumberActions];
    float initial_energy;
    int initial_marker;
    int initial_mutations;
    Matrix* initial_brain;
    int max_age;
    int total_steps;
    int next_id;

    Matrix weight_mask;

    int accuracy;

    bool mutation_color_drift;
    float mutation_prob;
    float mutation_sd;

    Occupant** occupant_grid;

    // Paint
    void Paint(QPainter& painter);
    void Paint(QImage& image);

    // State change
    Grid();
    virtual void Reset();
    ~Grid();

    // Setup
    void SetupMask(bool);
    void SetupActions();
    void SetSize(int rws, int cls);

    // Low-level position
    Pos Wrap(Pos pos) { return pos.Wrap(rows, cols); }
    Pos RandomCell();
    Pos EmptyCell();
    Pos FairCell();

    // Creat Management
    Creat& _AddCreat(Pos pos, int orient=0);
    Creat& AddCreatAt(Pos pos, int orient=0);
    void AddCreats(int number, bool fairly);
    void RemoveOccupants();
    Occupant* LookupOccupantByID(int id);
    Creat* FindCreat(int marker);
    inline Occupant*& OccupantAt(Pos pos)
    { return occupant_grid[pos.row * cols + pos.col]; }
    inline Creat* CreatAt(Pos pos)
    { return dynamic_cast<Creat*>(occupant_grid[pos.row * cols + pos.col]); }

    // Scoring
    Matrix FindDominantGenome();
    float CompeteScore(Matrix& a, Matrix& b);
    int CountCreatsByMarker(int marker);

    void Step();

    // Convenience functions
    void Run(int steps, int report=0);
    void Report();
    Matrix Evolve(int steps);

    // Creature vision functions
    float CreatKernel(Pos pos, int dir);
    float EnergyKernel(Pos pos, int dir);
};

float CompeteFunction(int numa, int numb);

#endif
