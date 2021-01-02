#include "ant.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <random>
using namespace std;
#define WORKER_APPETITE 1


static const inline double std_normal()
{
    random_device rd;
    mt19937_64 gen = mt19937_64(rd());
    normal_distribution<double> dis(0, 1);  // default [0, 100)
    return bind(dis, gen)();                // bind and call
}

// Go to destination for single step
static inline void go(pos_t &curr, const pos_t &dest)
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

namespace detail
{
void walk(Ant *me, pos_t oriented)
{
    auto &curr_pos = me->at();
    auto my_map = me->get_map();

    auto get_max_obj = [](vector<MapObj> &&v) {
        auto max_item = v[0];
        for (auto i : v)
            if (i.value > max_item.value)
                max_item = i;
        return make_pair(pos_t(), max_item);
    };

    // Scan near by
    auto find_near = [&]() {
        auto a = my_map->get_at(pos_t(curr_pos.x + 1, curr_pos.y));
        auto b = my_map->get_at(pos_t(curr_pos.x + 1, curr_pos.y + 1));
        auto c = my_map->get_at(pos_t(curr_pos.x, curr_pos.y + 1));

        if (a.type == FOOD)
            return make_pair(pos_t(curr_pos.x + 1, curr_pos.y), a);
        else if (b.type == FOOD)
            return make_pair(pos_t(curr_pos.x + 1, curr_pos.y + 1), b);
        else if (c.type == FOOD)
            return make_pair(pos_t(curr_pos.x, curr_pos.y + 1), c);
        else
            return get_max_obj({a, b, c});
    };

    //
    auto [near_where, near_what] = find_near();
    if (near_what.type == FOOD) {
        go(curr_pos, near_where);
        cout << "FOOD" << endl;
    } else if (abs(std_normal()) < 1) {
        // follow other PHEROMONE
        go(curr_pos, near_where);
        cout << "PHEROMONE" << endl;
    } else {
        go(curr_pos, curr_pos + oriented);
        cout << "WONDER" << endl;
    }
    cout << "============================" << endl;
    cout << "DDD" << curr_pos.x << "," << curr_pos.y << endl;
    cout << "============================" << endl;
}

};  // namespace detail

static inline STATUS __alive_handler(Ant *me)
{
    // Check and set
    if (me->get_step() == 0 || me->get_energy() == 0)
        me->set_live_status(STATUS::DEAD);

    return me->get_live_status();
}

Ant::Ant(LocalMap *_map, pos_t _my_home)
{
    this->job = make_unique<Worker>(this);
    this->myMap = _map;
    this->home_pos = _my_home;
}

pos_t &Ant::at()
{
    return this->pos;
}
void Ant::set_step(int _step)
{
    this->step = _step;
}
const int Ant::get_step() const
{
    return this->step;
}
void Ant::set_job(unique_ptr<Job> &&_job)
{
    this->job = move(_job);
}
void Ant::set_map(LocalMap *_myMap)
{
    this->myMap = _myMap;
}
LocalMap *Ant::get_map() const
{
    return this->myMap;
}
int Ant::get_energy() const
{
    return this->energy;
}
void Ant::set_energy(int _value)
{
    this->energy = _value;
}
STATUS Ant::get_live_status() const
{
    return this->is_alive;
}
void Ant::set_live_status(STATUS status)
{
    this->is_alive = status;
}
pos_t &Ant::home()
{
    return this->home_pos;
}

void info(Ant *a)
{
    cerr << "pos: \t(" << a->at().x << ", " << a->at().y << ")" << endl
         << "home: \t(" << a->home_pos.x << ", " << a->home_pos.y << ")" << endl
         << "step: \t" << a->step << endl
         << "energy: " << a->energy << endl
         << "alive: \t" << static_cast<bool>(a->is_alive) << endl;
    a->job->info();
}

void Worker::work()
{
    if (is_go_to_find_food && my_food.type == EMPTY)
        find_food();
    else if (my_food.type == EMPTY)
        my_food = pick_food();
    else
        return_home();
}

void Worker::put_pheromone(pos_t pos)
{
    me->get_map()->merge(pos, MapObj(100, PHEROMONE));
}

// Be careful! Here is eat the food from my hand
// Not pick up the food from env.
int Worker::get_food()
{
    if (my_food.type == FOOD) {
        my_food.value--;
        me->set_energy(me->get_energy() + WORKER_APPETITE);
    }
    return WORKER_APPETITE;
}

void Worker::alive_handler()
{
    me->set_live_status(__alive_handler(me));
}

void Worker::find_food()
{
    me->set_step(me->get_step() - 1);
    auto my_map = me->get_map();
    auto curr_pos = me->at();
    cerr << curr_pos;
    if (my_map->get_at(curr_pos).type != FOOD)
        detail::walk(me, this->oriented);
    else  // Next time will run `pick_food()`
        is_go_to_find_food = false;
}

MapObj Worker::pick_food()
{
    auto my_map = me->get_map();
    // FIXME: have race condition here
    MapObj picked_up = my_map->get_at(me->at());
    my_map->merge(me->at(), MapObj(-picked_up.value, FOOD));
    return picked_up;
}

void Worker::put_food(pos_t pos)
{
    auto my_map = me->get_map();
    my_map->merge(pos, my_food);
    my_food.clean();
}

void Worker::return_home()
{
    me->set_step(me->get_step() - 1);
    if (my_food.value <= 0) {
        // I had eaten the food when I take it back.
        my_food.clean();
        is_go_to_find_food = true;
    }
    go(me->at(), me->home());
    cout << "return home" << endl;

    if (me->at() == me->home()) {
        put_food(me->home());
        is_go_to_find_food = true;
    }
}

Worker::Worker(Ant *_me = nullptr) : me(_me)
{
    pos_t __proto_oriented(0, 1);
    if (std_normal() > 0)
        swap(__proto_oriented.x, __proto_oriented.y);
    oriented = __proto_oriented;
}

void Worker::info()
{
    cerr << "go \t" << ((is_go_to_find_food) ? "food" : "home") << endl
         << "my \t" << my_food.type << " " << my_food.value << endl
         << "oriented (" << oriented.x << ", " << oriented.y << ")\n";
}