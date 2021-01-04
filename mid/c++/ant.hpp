#ifndef __ANT_HPP__
#define __ANT_HPP__

#include "job.hpp"
#include "map.hpp"
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour
#define MAXSTEP 500
#define MAXENERGY 550

enum class STATUS : bool {
    DEAD,
    ALIVE,
};

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

void go(pos_t &curr, const pos_t &dest);

#endif