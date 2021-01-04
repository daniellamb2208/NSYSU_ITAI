#include "ant.hpp"
#include <iostream>
#include <memory>
#include "Worker.hpp"
using namespace std;

// Go to destination for single step
void go(pos_t &curr, const pos_t &dest)
{
    auto diff = dest - curr;
    // +1 prevent 0/0
    curr.x +=
        (abs(diff.x) > abs(diff.y)) ? ((diff.x | 1) / (abs(diff.x | 1))) : 0;
    curr.y +=
        (abs(diff.x) <= abs(diff.y)) ? ((diff.y | 1) / (abs(diff.y | 1))) : 0;

    // Overflow condition, move one step, so if overflow is occurred
    // previous position most be 0
    if (curr.x == -1)
        curr.x = HEIGHT - 1;
    if (curr.y == -1)
        curr.y = WIDTH - 1;
    if (curr.x >= HEIGHT)
        curr.x = 0;
    if (curr.y >= WIDTH)
        curr.y = 0;
}

Ant::Ant(LocalMap *_map, pos_t _my_home)
{
    this->job = make_unique<Worker>(this);
    this->myMap = _map;
    this->home_pos = _my_home;
}

void info(const Ant *a)
{
    // cerr << "In main: " << a << endl;
    cerr << "pos: \t(" << a->at().x << ", " << a->at().y << ")" << endl
         << "home: \t" << a->home() << endl
         << "step: \t" << a->get_step() << endl
         << "energy: " << a->get_energy() << endl
         << "alive: \t" << static_cast<bool>(a->get_live_status()) << endl;
    a->job->info();
}
