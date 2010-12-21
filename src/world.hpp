#ifndef GRID_HPP
#define GRID_HPP

#include "metaclass.hpp"
#include "pos.hpp"
#include "misc.hpp"
#include "matrix.hpp"

#include <iostream>
#include <list>

enum InteractionMode
{
    NoInteraction,
    Penalty,
    Attack,
    Zombie,
    Parasitism,
    Predation,
    Shoving,
    MutualCooperation,
    AltruisticCooperation,
    GeneExchange,
    GeneGive,
    GeneReceive,
    GeneSymmetric
};

class Occupant;
class QPainter;
class QImage;
class World;
class Creat;
typedef void (Creat::*CreatFunc)();

class Occupant : public Object
{
  public:

    Occupant();

    Occupant* next; // next occupant on this grid position
    World* grid;
    Pos pos;
    Pos last_pos;
    float signature;
    int id;
    bool solid;

    virtual void Interact(Creat& c);
    virtual void Update();

    virtual void Reset();
    virtual void __Remove();
    void AssignID();

    void Attach(World& grid, Pos pos);
    void Remove();
    void RemoveFromLL();
    void Move(Pos p);
    void MoveRandom();
};

std::ostream& operator<<(std::ostream& s, Occupant*& o);
std::istream& operator>>(std::istream& i, Occupant*& o);

// Creat depends on Occupant
#include "creat.hpp"

class World : public Object
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

    bool enable_mutation;
    bool enable_respawn;

    int path_energy;
    bool record_lineages;

    float action_cost[16];
    CreatFunc action_lookup[16];
    int initial_energy;
    int birth_cost;
    int initial_marker;
    int initial_mutations;
    Matrix* initial_brain;
    int max_age;
    int total_steps;
    int next_id;

    int accuracy;
    int neural_net_iterations;

    bool mutation_color_drift;
    float mutation_prob;
    float mutation_sd;
    int jump_range;

    Occupant** occupant_grid;

    bool hooks_enabled;

    // State change
    World();
    virtual void Reset();
    ~World();

    // Setup
    void SetupActions();
    void SetSize(int rws, int cls);

    // Low-level position
    inline Pos Wrap(Pos pos) { return pos.Wrap(rows, cols); }
    Pos RandomCell();
    Pos EmptyCell();
    Pos FairCell();

    // Creat Management
    Creat& _AddCreat(Pos pos, int orient=0);
    Creat& AddCreatAt(Pos pos, int orient=0);
    void AddCreats(int number, bool fairly);
    void RemoveOccupants();

    Occupant* SolidOccupantAt(Pos pos);
    Creat* CreatAt(Pos pos);
    Occupant* LookupOccupantByID(int id);
    Creat* FindCreat(int marker);
    inline Occupant*& OccupantAt(Pos pos) { return occupant_grid[pos.row * cols + pos.col]; }

    // Scoring
    Matrix FindDominantGenome();
    float CompeteScore(Matrix& a, Matrix& b);
    int CountCreatsByMarker(int marker);

    void ColorClusters();

    void Step();

    // Convenience functions
    void Run(int steps, int report=0);
    void Report();
    Matrix Evolve(int steps);

    // Creature vision functions
    float CreatKernel(Pos pos, int dir);
    float EnergyKernel(Pos pos, int dir);
    float DirKernel(Pos pos, int dir);
};

float CompeteFunction(int numa, int numb);

#endif
