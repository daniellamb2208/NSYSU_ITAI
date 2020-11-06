#include "map.hpp"
#include <array>
#include <functional>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

// Separate `value` into `num` parts by normal distribution
static vector<double> separateNormally(size_t num, double value)
{
    vector<double> result;
    // TODO: separate `value` into `num` parts by normal distribution
    for (size_t i = 0; i < num; i++)
        result.at(i) = value / num;  // Use uniform currently
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
    this->totalFoods = 0;
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
    double preValue = get_at(pos).value;
    (this->arr.at(pos.x).at(pos.y)).store(thing);

    // Add to objPool, just overwrite it
    // It's needless to free orig obj
    this->objPool[thing.type][pos] = thing;

    // Update objPool
    double postValue = get_at(pos).value;
    double origTotalFoods = this->totalFoods.load();
    this->totalFoods.store(origTotalFoods + (postValue - preValue));
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    // me is a rvalue of MapObj
    auto me = this->get_at(pos);
    // Put at home or type is same
    if (me.type == HOME || _source.type == me.type)
        me.value += _source.value;

    // one is FOOD
    // FOOD + PHEROMONE = FOOD, all the PHEROMONE will lost
    // Bitwise operation, don't touch it if you don't know.
    switch (me.type | _source.type) {
    case (FOOD | PHEROMONE):
        me.value = (me.type & FOOD) * me.value;
        _source.value = (_source.type & FOOD) * _source.value;
    case (FOOD):
        me.type = FOOD;
        break;
    case (PHEROMONE):
        me.type = PHEROMONE;
        break;
    default:
        throw "TypeError: merge";
        break;  // Undefined behavior, impossible execute here
    }
    me.value += _source.value;
    // Give `put_at` to process the objPool problem
    this->put_at(pos, me);
}

pos_t LocalMap::findClosest(const pos_t &currentPos, char type)
{
    // Designated initializers, after C++20
    pos_t pos = {.x = HEIGHT, .y = WIDTH};
    auto minLen = INFINITY;

    // C++17 structured bindings
    for (auto &[_pos, obj] : this->objPool[type]) {
        double len = currentPos - _pos;
        if (len < minLen) {
            minLen = len;
            pos = _pos;
        }
    }

    return pos;
}

// Default callee
void foodGenerator(LocalMap &_map)
{
    auto value = getRand() * MAX_FOOD * 0.03;  // Up to 3% of MAX

    // Designated initializers, after C++20
    pos_t pos = {.x = int(getRand() * HEIGHT), .y = int(getRand() * WIDTH)};

    size_t num = getRand() * 50;  // 50 food? I guess.
    foodGenerator(pos, num, value, _map);
}

// Separeate foods uniformly
void foodGenerator(pos_t _pos, size_t num, double value, LocalMap &_map)
{
    // FIXME: No, we cannot add them all
    if (value + _map.totalFoods.load() > MAX_FOOD)
        return;
    auto values = separateNormally(num, value);
    for (auto i : values)
        _map.merge(_pos, MapObj(i));
}