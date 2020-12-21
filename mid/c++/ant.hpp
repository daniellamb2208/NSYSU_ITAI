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
    bool is_alive = true;


    friend class Job;  // Make Job could access *this directly
    unique_ptr<Job> job = nullptr;

public:
    Ant() = delete;
    Ant(int job_code);  // set job by number,
    Ant(Job *_job = nullptr) : job(_job) {}
    ~Ant() = default;
    pos_t &at() { return this->pos; }
    void set_step(int _step) { this->step = _step; }
    const int get_step() const { return this->step; }
    void set_job(unique_ptr<Job> &&_job) { this->job = move(_job); }
    void set_map(LocalMap *_myMap) { this->myMap = _myMap; }
    LocalMap *get_map() { return this->myMap; }
    void set_queen_pos(Ant *queen) { this->my_queen_pos = queen->at(); }
    pos_t get_queen_pos() { return this->my_queen_pos; }
    int get_energy() const { return this->energy; }
    void set_energy(int _value) { this->energy = _value; }
    Job_Code get_job_code() { return this->my_job_code; }
    bool get_live_status() { return this->is_alive; }
    void set_live_status(bool status) { this->is_alive = status; }

    Ant &operator=(Ant &&) = default;
};

// -----------------

class Job
{
protected:
    // Including walk, job, management
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
    // Set live status in each work being done
    virtual void alive_handler() = 0;

public:
    virtual ~Job() {}
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
        alive_handler();
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
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    int get_food() final;
    void alive_handler() final;
    void remove_dead_ant();

    // Warning: Queen's my_queen_pos is undefined!

public:
    Queen(Ant *_me = nullptr) : me(_me) {}
    ~Queen() = default;
    vector<Ant> &get_slave() { return this->my_slave; };
};

class Male : public Job
{
    Ant *me;
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    int get_food() final;
    void alive_handler() final;

public:
    Male(Ant *_me = nullptr) : me(_me) {}
    ~Male() = default;
};

class Virgin : public Job
{
    Ant *me;
    vector<Ant> tmp_slave;
    pos_t new_home;
    void work() final;
    // Eat nothing
    void eat() final {}
    int get_food() final { return 0; }
    void alive_handler() final;

public:
    Virgin(Ant *_me = nullptr) : me(_me) {}
    ~Virgin() = default;
    // FIXME: there is another issue about moving the home
    // Where is the slave of this Virgin or new queen's home?
    // The home is not the Virgin or queen's place, and need to
    // remove the label of home on the map
    void set_new_home(pos_t pos) { this->new_home = pos; }
};


class Worker : public Job
{
    bool is_go_to_find_food = true;
    MapObj my_food = MapObj();
    Ant *me;

    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    void put_pheromone(pos_t pos);
    int get_food() final;
    void alive_handler() final;
    void find_food();
    MapObj pick_food();
    void return_home();

public:
    Worker(Ant *_me = nullptr) : me(_me) {}
    ~Worker() = default;
};

class Soldier : public Job
{
    Ant *me;
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    int get_food() final;
    void alive_handler() final;

public:
    Soldier(Ant *_me = nullptr) : me(_me) {}
    ~Soldier() = default;
};


#endif