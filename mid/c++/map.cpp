#include "map.hpp"
#include <array>
#include <functional>
#include <random>
#include <vector>
using namespace std;

// Separate `value` into `num` parts by normal distribution
static vector<double> separateNormally(size_t num, double value)
{
    vector<double> result;
    // TODO: separate `value` into `num` parts by normal distribution
    return result;
}

// Return [0,1) double
inline double getRand()
{
    random_device rd;
    mt19937 gen = mt19937(rd());
    uniform_real_distribution<> dis(0, 1);  // return double
    return bind(dis, gen)();
}

LocalMap::LocalMap()
{
    this->t = thread(sync);
}

LocalMap::~LocalMap()
{
    this->t.join();
}

MapObj LocalMap::get_at(pos_t pos)
{
    return (this->arr.at(pos.x).at(pos.y)).load();
}

void LocalMap::put_at(pos_t pos, MapObj thing)
{
    (this->arr.at(pos.x).at(pos.y)).store(thing);
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    // me is a lvalue of MapObj
    auto me = this->arr.at(pos.x).at(pos.y).load();
    // Put at home or type is same
    if (me.type == HOME || _source.type == me.type) {
        me.value += _source.value;
    } else {
        // one is FOOD, one is PHEROMONE
        // FOOD + PHEROMONE = FOOD, all the PHEROMONE will lost
        me.value = (_source.type == FOOD) ? _source.value : me.value;
        me.type = FOOD;
    }
    this->arr.at(pos.x).at(pos.y).store(me);
}

pos_t findClosest(pos_t currentPos, int type)
{
    // TODO: implement this function, might create a TREE in `LocalMap` class
    return pos_t();
}

// Default callee
void foodGenerator(LocalMap &_map)
{
    auto value = getRand() * MAX_FOOD * 0.03;  // Up to 3% of MAX

    // Designated initializers, after C++20
    pos_t pos = {.x = getRand() * HEIGHT, .y = getRand() * WIDTH};

    size_t num = getRand() * 50;  // 50 food? I guess.
    foodGenerator(pos, num, value, _map);
}

// Separeate foods uniformly
void foodGenerator(pos_t _pos, size_t num, double value, LocalMap &_map)
{
    // TODO: calculate the total foods(might use some tree)
    if (value + __TOTAL_FOOD__ > MAX_FOOD)
        return;
    auto values = separateNormally(num, value);
    for (auto i : values)
        _map.merge(_pos, MapObj(i));
}