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

pos_t pos_t::operator+(const pos_t other) const
{
    return pos_t(x + other.x, y + other.y);
}

pos_t pos_t::operator-(const pos_t other)
{
    this->x -= other.x;
    this->y -= other.y;
    return *this;
}

pos_t pos_t::operator-(const pos_t other) const
{
    return pos_t(x - other.x, y - other.y);
}

pos_t pos_t::operator*(double times)
{
    this->x *= times;
    this->y *= times;
    return *this;
}
ostream &operator<<(ostream &_out, const pos_t pos)
{
    _out << "(" << pos.x << ", " << pos.y << ") ";
    return _out;
}

MapObj::MapObj(double _value, char _type) : value(_value), type(_type) {}

MapObj MapObj::operator+(MapObj other)
{
    auto ffs = __builtin_ffs(this->type | other.type);
    auto merge_type = 1 << (ffs - 1);

    auto partner_ptr = (merge_type == other.type) ? this : (&other);
    auto pivot_ptr = (partner_ptr == this) ? (&other) : this;
    switch (merge_type) {
    case HOME:
    case FOOD:
        this->value = pivot_ptr->value + (((partner_ptr->type == FOOD) ||
                                           (partner_ptr->type == HOME))
                                              ? partner_ptr->value
                                              : 0);
        break;
    case PHEROMONE:
        this->value = pivot_ptr->value + partner_ptr->value;
        break;
    case EMPTY:
    default:
        break;
    }
    this->type = merge_type;
    if (this->value < 0)
        this->clean();
    return *this;
}

void LocalMap::sync()
{
    auto proc_unit = [&](MapObj obj, auto &jter) {
        auto preValue = obj.value;
        if (obj.value < 0.03 && obj.type != HOME)
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
        this_thread::sleep_for(chrono::seconds(1));
    }
}

LocalMap::LocalMap(bool disable_sync = false) : tot_foods(0)
{
    // Initialize arr
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++)
            arr.at(i).at(j) = MapObj();

    if (!disable_sync)
        this->t = thread(bind(&LocalMap::sync, this));
}

LocalMap::~LocalMap()
{
    if (t.joinable())
        this->t.join();
}

MapObj LocalMap::get_at(pos_t pos)
{
    MapObj obj;
    try {
        obj = this->arr.at(pos.x).at(pos.y).load();
    } catch (const std::exception &e) {
        throw e;
    }
    return obj;
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
        this->merge(pos, MapObj(i, FOOD));
    }
}

array<array<pair<double, char>, WIDTH>, HEIGHT> LocalMap::show()
{
    array<array<pair<double, char>, WIDTH>, HEIGHT> canvax;
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++) {
            auto obj = get_at(pos_t(i, j));
            canvax.at(i).at(j) = make_pair(obj.value, obj.type);
        }
    // Return 2-dimension array of double
    // You should cast the type to char by yourself(if you want type)
    return canvax;
}