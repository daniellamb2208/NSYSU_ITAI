#ifndef __ANT_HPP__
#define __ANT_HPP__

#include <unistd.h>
#include <vector>
#include "map.hpp"
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour
#define STEP 1
#define CHILDREN_BASE 30
#define MAXSTEP 1500
#define MAXENERGY 1500
#define SLEEP_DURATION 100  // micro_seconds
#define PHEROMONE_FREQUENCY 1
#define QUEEN_SLEEP 10

/**
 *                   -----------
 *                  |    Ant    |
 *                   -----------
 *          A generic ant with strategy pattern
 *
 */

enum class Job_Code : int {
    queen,
    virgin,
    male,
    solder,
    worker,
    End,  // Could add more job of ants, before End
};

class Job;

class Ant
{
protected:
    Job_Code my_job_code = Job_Code::worker;
    LocalMap *myMap = nullptr;
    // My position
    // Every steps should udpate this pos
    pos_t pos = pos_t();
    pos_t my_queen_pos = pos_t();
    // Descent form threshold(MAXSTEP), will die if count to zero.
    int step = MAXSTEP;
    // Descenting energy, can be increased by eat food, but will die if count to
    // zero.
    int energy = MAXENERGY;


    friend class Job;  // Make Job could access *this directly
    Job *job = nullptr;

public:
    Ant() = delete;
    Ant(int job_code);  // set job by number,
    Ant(Job *_job = nullptr) : job(_job) {}
    ~Ant() { delete this->job; }
    pos_t &at() { return this->pos; }
    void set_step(int _step) { this->step = _step; }
    const int get_step() const { return this->step; }
    void set_job(Job *_job)
    {
        delete this->job;
        this->job = _job;
    }
    void set_map(LocalMap *_myMap) { this->myMap = _myMap; }
    LocalMap *get_map() { return this->myMap; }
    void set_queen_pos(Ant *queen) { this->my_queen_pos = queen->at(); }
    pos_t get_queen_pos() { return this->my_queen_pos; }
    int get_energy() const { return this->energy; }
    void set_energy(int _value) { this->energy = _value; }
    Job_Code get_job_code() { return this->my_job_code; }
};

// -----------------

class Job
{
protected:
    virtual void work() = 0;
    // Eat the food which it get
    virtual void eat() = 0;
    // Different ant eat different food
    // For example: Worker eat its own food first
    //              Bet the Queen, Solder, Male, Virgin would eat the food
    //              from home first
    // This function will consume food from env. to itself.
    // If not found, return 0.
    virtual int get_food() = 0;

public:
    virtual ~Job() {}
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
    }
};

class Queen : public Job
{
    vector<Ant> my_slave;
    Ant *me;
    void do_sleep() { sleep(QUEEN_SLEEP); }
    bool find_male();
    bool bleed() { return find_male(); }
    void pregnant(Job_Code _gift);  // job code with gift
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - 100); }
    int get_food() final;

    // Warning: Queen's my_queen_pos is undefined!

public:
    Queen(Ant *_me = nullptr) : me(_me) {}
    ~Queen();
};

class Male : public Job
{
    Ant *me;
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - 80); }
    int get_food() final;

public:
    Male(Ant *_me = nullptr) : me(_me) {}
    ~Male();
};

class Virgin : public Job
{
    Ant *me;
    void work() final;
    // Eat nothing
    void eat() final {}
    int get_food() final {}

public:
    Virgin(Ant *_me = nullptr) : me(_me) {}
    ~Virgin();
};


class Worker : public Job
{
    Ant *me;
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - 1); }
    void put_pheromone(pos_t pos);
    int get_food() final;

public:
    Worker(Ant *_me = nullptr) : me(_me) {}
    ~Worker();
};

class Soldier : public Job
{
    Ant *me;
    void work() final;
    void eat() final;
    int get_food() final;

public:
    Soldier(Ant *_me = nullptr) : me(_me) {}
    ~Soldier();
};


#endif