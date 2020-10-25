#include "map.hpp"
#include <vector>
using namespace std;

static const double DISCOUNT_LAMBDA = 1.1;

LocalMap::LocalMap(pos_t pos)
{
    this->vec = vector<vector<MapObj>>(pos.x, vector<MapObj>(pos.y, MapObj()));
    this->t = thread(sync);
}

LocalMap::~LocalMap()
{
    this->t.join();
}

MapObj &LocalMap::at(pos_t pos)
{
    return this->vec.at(pos.x).at(pos.y);
}

void LocalMap::sync()
{
    for (auto i : vec)
        for (auto j : i) {
            if (j.value < 0.05)
                j.clean();
            if (j.type == PHEROMONE)
                j.pheromone /= DISCOUNT_LAMBDA;
        }
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    if (_source.type == this->vec.at(pos.x).at(pos.y).type) {
        this->vec.at(pos.x).at(pos.y).value += _source.value;
        return;
    }

    // one is FOOD, one is PHEROMONE
    auto me = this->vec.at(pos.x).at(pos.y).value;
    me = (_source.type == FOOD) ? _source.value : me;
    this->vec.at(pos.x).at(pos.y).type = FOOD;
}