#ifndef __ANT_HPP__
#define __ANT_HPP__

#include <vector>
#include "map.hpp"
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour 
#define STEP 1
#define CHILDREN_BASE 30
#define MAXSTEP 1500
#define SLEEP_DURATION 0  // micro_seconds
#define PHEROMONE_FREQUENCY 1

/**
 * Inheritance Graph:
 *                   -----------
 *                  |    Ant    |
 *                   -----------
 *                   /         \
 *                  /           \
 *                 /             \
 *            -----------         -----------
 *           |   Breed   |       | Non-breed |
 *            -----------         -----------
 *            /        \            /      \
 *        ------    -------      -------    ------
 *       | Male |  | Queen |    |Soldier|  |Worker|
 *        ------    -------      -------    -------
 *                     |
 *                  --------
 *                 | Virgin |
 *                  --------
 *
 */

// TODO: Use `Queen` to control all slave ant

// Declare
class Queen;
class Male;
class Virgin;
class Soldier;
class Worker;

class Ant
{
protected:
    LocalMap *myMap;
    // My position
    // Every steps should udpate this pos
    pos_t pos;
    // Descent form threshold(MAXSTEP), will die if count to zero.
    int step;
    // An object on the ant.
    MapObj myObj;
    Queen *myQueen;

public:
    Ant() = default;
    ~Ant();
    pos_t &at();
    void setStep(int _step);
    const int getStep();
    // Call distructor
    void die();
    void sync();
    void setMap(LocalMap *_myMap);
    LocalMap *getMap();
    MapObj getObj();
    void setMyQueen(Queen *_myQueen);
    pos_t getQueenPos();
    virtual void job() = 0;
};

// ----------------

class BreedAnt : public Ant
{
protected:
    // Who has fertility, eat food
    double appetite;

public:
    BreedAnt() = default;
    BreedAnt(LocalMap *_myMap, double _appetite = 2);
    ~BreedAnt();
    void setAppetite(double);
    double getAppetite();
    void eat();
};

class NonBreedAnt : public Ant
{
public:
    NonBreedAnt() = default;
    NonBreedAnt(LocalMap *_myMap);
    ~NonBreedAnt();
    // Take current pos for some food
    void take(MapObj obj);
};

// ----------------

class Queen : public BreedAnt
{
protected:
    // FIXME: Bug here
    shared_ptr<vector<shared_ptr<Ant>>> slave;
    vector<shared_ptr<Queen>> *queenPtrPool;

public:
    Queen() = default;
    Queen(LocalMap *_myMap,
          pos_t _pos,
          vector<shared_ptr<Queen>> *queenPtrPool);
    ~Queen();
    // Consume some food then sleep
    int pregnant();
    // Loop eat, mate and pregnant
    void job();
    // find Male ant, in order to mate
    const bool findMale();
    void setQPP(vector<shared_ptr<Queen>> *_queenPtrPool);
    shared_ptr<vector<shared_ptr<Ant>>> &getSlave();
    pos_t getPos() { return this->pos; }
};

class Male : public BreedAnt
{
public:
    Male() = default;
    Male(LocalMap *_myMap);
    ~Male();
    void setAppetite(double _appetite);
    // Mate with queen or eat food
    void job();
    // If mated once, die
};

class Virgin : public Queen
{
public:
    Virgin() = default;
    Virgin(LocalMap *_myMap);
    ~Virgin();
    // Move some resource to create a new empire
};

class Soldier : public NonBreedAnt
{
    double portectDistance;

public:
    Soldier() = default;
    Soldier(LocalMap *_myMap);
    ~Soldier();
    void setProtectDistance(double pd);
    void job();
    // TODO: Protect queen
};

class Worker : public NonBreedAnt
{
public:
    Worker() = default;
    Worker(LocalMap *_myMap);
    ~Worker();
    void wandering();
    void putPheromone();
    void job();

private:
    // Once take food, record is there still remaining food
    // If remaining, give value true, and vice versa
    bool remaining;
};

#endif