#ifndef __WORKER_HPP__
#define __WORKER_HPP__

#include "job.hpp"

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
