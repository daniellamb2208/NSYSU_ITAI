#ifndef __QUEEN_HPP__
#define __QUEEN_HPP__

#include <vector>
#include "job.hpp"


// Queen must at home
class Queen : public Job
{
    Ant *me;
    vector<unique_ptr<Ant>> slave;
    LocalMap *my_map;

    void add_ant(int num);
    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    void alive_handler() final;
    void clean() final;  // Kill slaves

public:
    Queen(Queen &&q) {}
    Queen(Ant *_me);
    ~Queen() { clean(); }
    void info() final;
    vector<unique_ptr<Ant>> &get_slave() { return this->slave; }
};


#endif