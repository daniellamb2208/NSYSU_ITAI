#include "map.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <future>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

// Separate `value` into `num` parts by normal distribution
static vector<double> separateNormally(size_t num, double value)
{
    vector<double> result;
    result.resize(num);
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

void LocalMap::sync()
{
    // i for first loop, j for second loop,
    // iterator is the operand named [ij]ter
    for (auto iter = arr.begin(); iter != arr.end(); iter++) {
        for (auto jter = iter->begin(); jter != iter->end(); jter++) {
            auto obj = jter->load();

            auto preValue = obj.value;
            auto t = obj.type;
            pos_t pos(size_t(iter - arr.begin()), size_t(jter - iter->begin()));
            if (obj.value < 0.03) {
                obj.clean();
                // Update objPool if rem
                this->objPool[t].erase(pos);
            }
            // PHEROMONE will dissipate
            if (obj.type == PHEROMONE) {
                obj.value /= DISCOUNT_LAMBDA;
                this->objPool[t][pos] = obj;
            }
            jter->store(obj);
            // Update totalFoods
            auto origTotalFoods = totalFoods.load();
            totalFoods.store(origTotalFoods + jter->load().value - preValue);
        }
    }
}

LocalMap::LocalMap()
{
    // Initialize arr
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++) {
            MapObj obj;
            arr.at(i).at(j) = obj;
        }

    // Use wrapper function
    this->t = thread(bind(&LocalMap::sync, this));
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

void LocalMap::put_at(pos_t pos, MapObj obj)
{
    double preValue = get_at(pos).value;
    (this->arr.at(pos.x).at(pos.y)).store(obj);

    // Add to objPool, just overwrite it
    // It's needless to free orig obj
    this->objPool[obj.type][pos] = obj;

    // Update objPool
    double postValue = get_at(pos).value;
    double origTotalFoods = this->totalFoods.load();
    this->totalFoods.store(origTotalFoods + (postValue - preValue));
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    // `me` is a rvalue of MapObj
    auto me = this->get_at(pos);
    // Put at home or type is same
    if (me.type == HOME || _source.type == me.type)
        _source.value =
            (me.type + _source.type == HOME + PHEROMONE) ? 0 : _source.value;
    else {
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
            break;  // Undefined behavior, impossible to execute here
        }
    }
    me.value += _source.value;  // might write negative value in
    if (me.value < 0)
        me.clean();

    // Give `put_at` to process the objPool problem
    this->put_at(pos, me);
}

pos_t LocalMap::findClosest(const pos_t &currentPos, char type)
{
    // Designated initializers, after C++20
    pos_t pos(-1, -1);
    auto minLen = INFINITY;

    // C++17 structured bindings
    for (auto &[_pos, obj] : this->objPool[type]) {
        double len = abs(long(_pos.x) - long(currentPos.x)) +
                     abs(long(_pos.y) - long(currentPos.y));
        if (len < minLen) {
            minLen = len;
            pos = _pos;
        }
    }

    return pos;  // Find a object is closest in the `type`
}

// Default callee
void LocalMap::foodGenerator()
{
    auto value = int(getRand() * MAX_FOOD);  // Up to 15% of MAX

    size_t num = getRand() * 50;  // 50 food? I guess.
    foodGenerator(num, value);
}

// Separeate foods normally
void LocalMap::foodGenerator(size_t num, double value)
{
    if (value + this->totalFoods.load() > MAX_FOOD)
        return;
    auto values = separateNormally(num, value);

    // Because each `pos` obj is independent,
    // and each obj might have blocking IO from atomic operation
    // TODO: Parallelize it!
    for (auto i : values) {
        pos_t pos(size_t(getRand() * HEIGHT), size_t(getRand() * WIDTH));
        this->merge(pos, MapObj(i));
    }
}

array<array<MapObj, WIDTH>, HEIGHT> LocalMap::shotMap()
{
    array<array<MapObj, WIDTH>, HEIGHT> canvax;
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++)
            canvax.at(i).at(j) = get_at(pos_t(i, j));
    // Return 2-dimension array of MapObj
    return canvax;
}

array<array<double, WIDTH>, HEIGHT> LocalMap::show(bool mode)
{
    array<array<double, WIDTH>, HEIGHT> canvax;
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++) {
            auto obj = get_at(pos_t(i, j));
            canvax.at(i).at(j) =
                (double("0FPH"[int(obj.type)] * (!mode))) + (obj.value * mode);
        }
    // Return 2-dimension array of double
    // You should cast the type to char by yourself(if you want type)
    return canvax;
}