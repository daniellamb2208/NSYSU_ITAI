#include "Worker.hpp"
#include <functional>
#include <iostream>
#include <random>
#define WORKER_APPETITE 1

static const inline double std_normal()
{
    random_device rd;
    mt19937_64 gen = mt19937_64(rd());
    normal_distribution<double> dis(0, 1);  // default [0, 100)
    return dis(gen);                        // bind and call
}

static inline STATUS __alive_handler(Ant *me)
{
    // Check and set
    if (me->get_step() <= 0 || me->get_energy() <= 0)
        me->set_live_status(STATUS::DEAD);

    return me->get_live_status();
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
        return max_item;
    };

    // Scan near by
    auto find_near = [&]() {
        MapObj a, b, c;

        // Might ou-of-range while get a, b, c
        a = my_map->get_at(pos_t((curr_pos.x + 1) % HEIGHT, curr_pos.y));
        b = my_map->get_at(
            pos_t((curr_pos.x + 1) % HEIGHT, (curr_pos.y + 1) % WIDTH));
        c = my_map->get_at(pos_t(curr_pos.x, (curr_pos.y + 1) % WIDTH));

        if (a.type == FOOD)
            return make_pair(pos_t((curr_pos.x + 1) % HEIGHT, curr_pos.y), a);
        else if (b.type == FOOD)
            return make_pair(
                pos_t((curr_pos.x + 1) % HEIGHT, (curr_pos.y + 1) % WIDTH), b);
        else if (c.type == FOOD)
            return make_pair(pos_t(curr_pos.x, (curr_pos.y + 1) % WIDTH), c);
        else
            return make_pair(
                pos_t((curr_pos.x + 1) % HEIGHT, (curr_pos.y + 1) % WIDTH),
                get_max_obj({a, b, c}));
    };

    //
    auto [near_where, near_what] = find_near();
    // cerr << "Want to go: " << near_where << endl;
    if (near_what.type == FOOD) {
        go(curr_pos, near_where);
    } else if (abs(std_normal()) < 1) {
        // follow other PHEROMONE
        go(curr_pos, near_where);
    } else {
        go(curr_pos, curr_pos + oriented);
    }
}

};  // namespace detail


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
    // cerr << "In Ant: " << this->me << endl;
    me->set_step(me->get_step() - 1);
    auto my_map = me->get_map();
    auto curr_pos = me->at();
    // cerr << curr_pos << endl;
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
    put_pheromone(me->at());
    go(me->at(), me->home());


    if (me->at() == me->home()) {
        put_food(me->home());
        is_go_to_find_food = true;
    }
}

Worker::Worker(Ant *_me = nullptr) : me(_me)
{
    pos_t __proto_oriented(0, 1);
    if (std_normal() > -1)
        swap(__proto_oriented.x, __proto_oriented.y);
    oriented = __proto_oriented;
    my_food.clean();
}

void Worker::info()
{
    cerr << "go \t" << ((is_go_to_find_food) ? "food" : "home") << endl
         << "my \t" << (int) my_food.type << " " << my_food.value << endl
         << "oriented (" << oriented.x << ", " << oriented.y << ")\n";
}