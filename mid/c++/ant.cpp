#include "ant.hpp"
#include <memory>
using namespace std;
#define WORKER_APPETITE 1

// Go to destination for single step
static inline void go(pos_t &curr, const pos_t &dest)
{
    long int x_diff = dest.x - curr.x;
    long int y_diff = dest.y - curr.y;
    // +1 prevent 0/0
    curr.x +=
        (abs(x_diff) > abs(y_diff)) ? ((x_diff | 1) / abs(x_diff | 1)) : 0;
    curr.y +=
        (abs(x_diff) <= abs(y_diff)) ? ((y_diff | 1) / abs(y_diff | 1)) : 0;

    // Overflow condition, move one step, so if overflow is occurred
    // previous position most be 0
    if (curr.x & (1 << 31))
        curr.x = 0;
    if (curr.y & (1 << 31))
        curr.y = 0;
    if (curr.x >= HEIGHT)
        curr.x = HEIGHT - 1;
    if (curr.y >= WIDTH)
        curr.y = WIDTH - 1;
}

namespace Detail
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
        else {
            return get_max_obj({a, b, c});
        }
    };

    //
    auto [near_where, near_what] = find_near();
    if (near_what.type == FOOD) {
        go(curr_pos, near_where);
    } else if (abs(std_normal()) < 1) {
        // follow other PHEROMONE
        go(curr_pos, near_where);
    } else {
        go(curr_pos, curr_pos + oriented);
    }
}

};  // namespace Detail

static inline bool __alive_handler(Ant *me)
{
    // Check and set
    if (me->get_step() == 0 || me->get_energy() == 0)
        me->set_live_status(false);

    return me->get_live_status();
}

Ant::Ant(LocalMap *_map, pos_t _my_home)
{
    this->job = make_unique<Worker>(this);
    this->myMap = _map;
    this->home_pos = _my_home;
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
    __alive_handler(me);
}

void Worker::find_food()
{
    me->set_step(me->get_step() - 1);
    auto my_map = me->get_map();
    auto curr_pos = me->at();
    if (my_map->get_at(curr_pos).type != FOOD)
        Detail::walk(me, this->oriented);
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

void Worker::return_home()
{
    me->set_step(me->get_step() - 1);
    if (my_food.value == 0) {
        // I had eaten the food when I take it back.
        my_food.type = EMPTY;
        is_go_to_find_food = true;
    }
    go(me->at(), me->home());
}