#include "Queen.hpp"
#include <iostream>

#define INIT_CHILD 5
#define GEN_ANT_THRESHOLD 200

inline void show_info(const Ant *a)
{
    // Call namespace of ant
    info(a);
}

inline void Queen::add_ant(int num = 1)
{
    for (int i = 0; i < num; i++)
        this->slave.push_back(make_unique<Ant>(my_map, me->at()));
}

void Queen::work()
{
    // Let slaves go
    for (auto &i : slave) {
        i->job->do_job();
        show_info(i->get_me());
    }

    // Collect dead ants
    for (auto iter = slave.begin(); iter != slave.end();) {
        if (iter->get()->get_live_status() == STATUS::DEAD)
            iter = slave.erase(iter);
        else
            iter++;
    }

    // gen new ants
    auto num = my_map->get_at(pos_t()).value / GEN_ANT_THRESHOLD;
    my_map->merge(me->at(), MapObj((-1) * GEN_ANT_THRESHOLD * num, HOME));
    add_ant(num);
}

void Queen::alive_handler()
{
    // No food at home
    if (me->get_map()->get_at(me->at()).value > 0)
        return;

    me->set_live_status(STATUS::DEAD);

    // swap virgin out
    if (slave.size() > 0) {
        slave[0]->set_job(make_unique<Queen>(slave[0]->get_me()));
        slave[0]->set_energy(MAXENERGY);
        slave[0]->set_step(MAXSTEP);
        auto &receiver = slave[0]->job->get_slave();
        for (int i = 1; i < slave.size(); i++)
            receiver.push_back(move(slave[i]));
        // TODO: how to move to new home?
    }
}

// Remove all slave
void Queen::clean()
{
    for (auto iter = slave.begin(); iter != slave.end();)
        iter = slave.erase(iter);
}

Queen::Queen(Ant *_me = nullptr) : me(_me)
{
    for (int i = 0; i < INIT_CHILD; i++)
        add_ant();
    this->my_map = me->get_map();
    me->home() = pos_t(rand() % HEIGHT, rand() % WIDTH);
}

void Queen::info()
{
    cerr << "My food: \t" << my_map->get_at(me->at()).value << endl
         << "My slaves: \t" << slave.size() << endl;
}