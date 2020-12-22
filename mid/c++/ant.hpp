#ifndef __ANT_HPP__
#define __ANT_HPP__

#include <functional>
#include <random>
#include <vector>
#include "map.hpp"
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour
#define STEP 1
#define CHILDREN_BASE 30
#define MAXSTEP 1500
#define MAXENERGY 1500
#define SLEEP_DURATION 0  // micro_seconds
#define PHEROMONE_FREQUENCY 1

double std_normal()
{
    random_device rd;
    mt19937_64 gen = mt19937_64(rd());
    normal_distribution<double> dis(0, 1);  // default [0, 100)
    return bind(dis, gen)();                // bind and call
}

/**
 * All the ants are worker.
 *
 */
class Job;

class Ant
{
private:
    LocalMap *myMap = nullptr;
    // My position
    // Every steps should udpate this pos
    pos_t pos = pos_t();
    pos_t home_pos = pos_t();
    // Descent form threshold(MAXSTEP), will die if count to zero.
    int step = MAXSTEP;
    // Descenting energy, can be increased by eat food, but will die if
    // count to zero.
    int energy = MAXENERGY;
    bool is_alive = true;

protected:
    friend class Job;  // Make Job could access *this directly
    unique_ptr<Job> job = nullptr;

public:
    Ant() = delete;
    Ant(LocalMap *_map, pos_t _my_home);
    // Ant(int job_code);  // set job by number,
    // Ant(Job *_job = nullptr) : job(_job) {}
    ~Ant() = default;
    pos_t &at() { return this->pos; }
    void set_step(int _step) { this->step = _step; }
    const int get_step() const { return this->step; }
    void set_job(unique_ptr<Job> &&_job) { this->job = move(_job); }
    void set_map(LocalMap *_myMap) { this->myMap = _myMap; }
    LocalMap *get_map() { return this->myMap; }
    int get_energy() const { return this->energy; }
    void set_energy(int _value) { this->energy = _value; }
    bool get_live_status() { return this->is_alive; }
    void set_live_status(bool status) { this->is_alive = status; }
    pos_t &home() { return this->home_pos; }

    Ant &operator=(Ant &&) = default;
};

class Job
{
protected:
    // Including walk, job, management
    virtual void work() = 0;
    // Eat the food which it get
    virtual void eat() = 0;
    virtual int get_food() = 0;
    // Set live status in each work being done
    virtual void alive_handler() = 0;
    virtual void clean() = 0;

public:
    ~Job() { clean(); }
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
        alive_handler();
    }
};

class Worker : public Job
{
    bool is_go_to_find_food = true;
    MapObj my_food = MapObj();
    Ant *me;
    pos_t oriented;

    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    void put_pheromone(pos_t pos);
    int get_food() final;
    void alive_handler() final;
    void clean() final;
    void find_food();
    MapObj pick_food();
    void return_home();

public:
    Worker(Ant *_me = nullptr) : me(_me)
    {
        oriented = pos_t(std_normal() > 0, std_normal() > 0);
    }
};

#endif