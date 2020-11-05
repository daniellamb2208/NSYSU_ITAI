#include "map.hpp"
#include <array>
using namespace std;

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