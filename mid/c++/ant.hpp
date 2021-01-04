#ifndef __ANT_HPP__
#define __ANT_HPP__

#include "map.hpp"
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour
#define STEP 1
#define CHILDREN_BASE 30
#define MAXSTEP 500
#define MAXENERGY 550
#define SLEEP_DURATION 0  // micro_seconds
#define PHEROMONE_FREQUENCY 1

enum class STATUS : bool {
    DEAD,
    ALIVE,
};

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
    STATUS is_alive = STATUS::ALIVE;

protected:
    friend class Job;  // Make Job could access *this directly

public:
    unique_ptr<Job> job;

    Ant() = delete;
    Ant(LocalMap *_map, pos_t _my_home);
    Ant(const Ant &) = delete;
    Ant(Ant &&o)
        : myMap(o.myMap),
          pos(o.pos),
          home_pos(o.home_pos),
          step(o.step),
          energy(o.energy),
          is_alive(o.is_alive),
          job(move(o.job)){};
    Ant &operator=(const Ant &) = delete;
    Ant &operator=(Ant &&) = default;
    ~Ant() = default;

    pos_t &at() { return this->pos; }
    const pos_t &at() const { return this->pos; }
    void set_step(int _step) { this->step = _step; }
    const int get_step() const { return this->step; }
    void set_job(unique_ptr<Job> &&_job) { this->job = move(_job); }
    void set_map(LocalMap *_myMap) { this->myMap = _myMap; }
    LocalMap *get_map() const { return this->myMap; }
    int get_energy() const { return this->energy; }
    void set_energy(int _value) { this->energy = _value; }
    STATUS get_live_status() const { return this->is_alive; }
    void set_live_status(STATUS status) { this->is_alive = status; }
    pos_t &home() { return this->home_pos; }
    const pos_t &home() const { return this->home_pos; }
    const Ant *get_me() const { return this; };

    friend void info(const Ant *a);
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
    virtual ~Job() {}
    Job() = default;
    Job(Job &&j) = default;
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
        alive_handler();
    }
    virtual void info() {}
};

class Worker : public Job
{
    bool is_go_to_find_food = true;
    MapObj my_food = MapObj(0, EMPTY);
    Ant *me;
    pos_t oriented;

    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    void put_pheromone(pos_t pos);
    int get_food() final;
    void alive_handler() final;
    void clean() final{};
    void find_food();
    MapObj pick_food();
    void put_food(pos_t pos);
    void return_home();

public:
    Worker(Worker &&w)
        : is_go_to_find_food(w.is_go_to_find_food),
          my_food(w.my_food),
          me(w.me),
          oriented(w.oriented)
    {
    }
    Worker(Ant *_me);
    ~Worker() { clean(); }
    void info() final;
};

#endif