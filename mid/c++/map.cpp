#include "map.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

// Separate `value` into `num` parts by normal distribution
static vector<double> __sep_norm(size_t num, double value)
{
    vector<double> result;
    result.resize(num);
    // TODO: separate `value` into `num` parts by normal distribution
    for (size_t i = 0; i < num; i++)
        result.at(i) = value / num;  // Use uniform currently
    return result;
}

// Return [0,1) double
static inline double __get_rand()
{
    random_device rd;
    mt19937 gen = mt19937(rd());
    uniform_real_distribution<> dis(0, 1);  // return double
    return bind(dis, gen)();
}

double norm(const pos_t a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

double cos(const pos_t a, const pos_t b)
{
    return dot(a, b) / (norm(a) * norm(b));
}

double dot(const pos_t a, const pos_t b)
{
    return a.x * b.x + a.y * b.y;
}

bool operator<(const pos_t a, const pos_t b)
{
    return norm(a) < norm(b);
}

bool operator==(const pos_t a, const pos_t b)
{
    return (a.x == b.x) && (a.y == b.y);
}

bool operator!=(const pos_t a, const pos_t b)
{
    return !(a == b);
}

pos_t &pos_t::operator=(const pos_t src)
{
    memcpy(this, &src, sizeof(src));
    return *this;
}

pos_t pos_t::operator+(const pos_t other)
{
    this->x += other.x;
    this->y += other.y;
    return *this;
}

pos_t pos_t::operator-(const pos_t other)
{
    this->x -= other.x;
    this->y -= other.y;
    return *this;
}

pos_t pos_t::operator*(const double times)
{
    this->x *= times;
    this->y *= times;
    return *this;
}

MapObj MapObj::operator+(const MapObj other)
{
    auto _copy_other(other);
    // Put at home or type is same
    if (this->type == HOME || _copy_other.type == this->type)
        _copy_other.value = (this->type + _copy_other.type == HOME + PHEROMONE)
                                ? 0
                                : _copy_other.value;
    else {
        // FOOD + PHEROMONE = FOOD, all the PHEROMONE will lost
        // Bitwise operation, don't touch it if you don't know.
        switch (this->type | _copy_other.type) {
        case (FOOD | PHEROMONE):
            this->value = (this->type & FOOD) * this->value;
            _copy_other.value = (_copy_other.type & FOOD) * _copy_other.value;
        case (FOOD):
            this->type = FOOD;
            break;
        case (PHEROMONE):
            break;  // not Change
        default:
            throw runtime_error("TypeError: merge");
            break;  // Undefined behavior, impossible to be executed here
        }
    }
    this->value += _copy_other.value;  // might write negative value in
    if (this->value < 0)
        this->clean();
    return *this;
}

void LocalMap::sync()
{
    auto proc_unit = [&](MapObj obj, auto &jter) {
        auto preValue = obj.value;
        if (obj.value < 0.03)
            obj.clean();

        // PHEROMONE will dissipate
        if (obj.type == PHEROMONE)
            obj.value /= DISCOUNT_LAMBDA;

        // Update the array
        jter->store(obj);

        // Update tot_foods
        if (obj.type == FOOD) {
            auto orig_tot_foods = tot_foods.load();
            tot_foods.store(orig_tot_foods + obj.value - preValue);
        }
    };

    while (true) {
        for (auto iter = arr.begin(); iter != arr.end(); iter++)
            for (auto jter = iter->begin(); jter != iter->end(); jter++)
                proc_unit(jter->load(), jter);

        food_gen();
    }
}

LocalMap::LocalMap()
{
    // Initialize arr
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++)
            arr.at(i).at(j) = MapObj();

    // Use wrapper function
    this->t = thread(bind(&LocalMap::sync, this));
    this->tot_foods = 0;
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

    if (obj.type == FOOD) {
        double postValue = get_at(pos).value;
        double orig_tot_foods = this->tot_foods.load();
        this->tot_foods.store(orig_tot_foods + (postValue - preValue));
    }
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    auto me = this->get_at(pos);
    me = me + _source;
    this->put_at(pos, me);
}

// Default callee
void LocalMap::food_gen()
{
    auto value = int(__get_rand() * MAX_FOOD);

    size_t num = __get_rand() * 50;  // 50 food? I guess.
    food_gen(num, value);
}

// Separeate foods normally
void LocalMap::food_gen(size_t num, double value)
{
    if (value + this->tot_foods.load() > MAX_FOOD)
        return;
    auto values = __sep_norm(num, value);

    // Because each `pos` obj is independent,
    // and each obj might have blocking IO from atomic operation
    // TODO: Parallelize it!
    for (auto i : values) {
        pos_t pos(size_t(__get_rand() * HEIGHT), size_t(__get_rand() * WIDTH));
        this->merge(pos, MapObj(i));
    }
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