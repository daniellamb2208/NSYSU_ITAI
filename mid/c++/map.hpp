#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <array>
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#define HEIGHT 15  // test
#define WIDTH 20
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH)

const double DISCOUNT_LAMBDA = 1.1;
using namespace std;

enum { EMPTY, FOOD, PHEROMONE, HOME };

typedef struct _p {
    size_t x;
    size_t y;

    _p() : x(0), y(0) {}
    _p(size_t _x, size_t _y) : x(_x), y(_y) {}
    _p(double _x, double _y) : x(long(_x)), y(long(_y)) {}
    _p(int _x, int _y) : x(_x), y(_y) {}
    // Length of distance
    friend double operator-(const struct _p(&a), const struct _p(&b))
    {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }
    friend bool operator<(const struct _p(&a), const struct _p(&b))
    {
        return (a.x * a.x) + (a.y * a.y) < (b.x * b.x) + (b.y * b.y);
    }
    friend bool operator!=(const struct _p(&a), const struct _p(&b))
    {
        return (a.x != b.x && a.y != b.y);
    }
    friend bool operator==(const struct _p(&a), const struct _p(&b))
    {
        return (a.x == b.x && a.y == b.y);
    }
    _p &operator=(const _p a)
    {
        this->x = a.x;
        this->y = a.y;
        return *this;
    }
    _p operator+(const _p &other)
    {
        return _p(this->x + other.x, this->y + other.y);
    }
} pos_t;

class MapObj
{
public:
    MapObj() noexcept : value(0), type(EMPTY) {}
    MapObj(double _value, char _type = FOOD) : value(_value), type(_type) {}
    void clean()
    {
        this->type = EMPTY;
        this->value = 0;
    }
    union {
        double pheromone;
        double food;
        double value;
    };
    char type;
};

class LocalMap
{
private:
    atomic<double> totalFoods;
    // This map is a static 2D array of atomic obj
    array<array<atomic<MapObj>, WIDTH>, HEIGHT> arr;
    // This is a naive way to find the closest obj
    // Store all PHEROMONE and FOOD's position to this obj pool
    // And brute force linear searching all obj
    // Access by objPool[FOOD][pos];
    map<char, map<pos_t, MapObj>> objPool;
    thread t;
    // Sync this map obj, create a thread to run it in background
    void sync();

public:
    LocalMap();
    ~LocalMap();

    // Get the obj
    MapObj get_at(pos_t pos);
    // Put it forcibly
    void put_at(pos_t pos, MapObj obj);

    // Merge the income obj at that place
    // might be pheromone, food or empty
    // EMPTY + ANY_TYPE = ANY_TYPE
    void merge(pos_t pos, MapObj _source);

    // Generate some foods in random time, place, numbers and value by default
    // Guarantee no overcommit food(If require more then MAX, will be ignored)
    void foodGenerator(size_t num, double value);
    // Default call this function
    void foodGenerator();

    // Find a object is closest in the `type`
    // There is some algorithm called "Space Partitioning"
    // https://stackoverflow.com/questions/28154450/find-nearest-object-in-2d-can-it-be-optimised-below-on/28155167#28155167
    pos_t findClosest(const pos_t &currentPos, char type);

    array<array<MapObj, WIDTH>, HEIGHT> shotMap();
    // `mode` 0: Array of types of obj,
    //      you should cast the type to char by yourself
    // `mode` 1: Array of value of obj
    array<array<double, WIDTH>, HEIGHT> show(bool mode = 0);
};

inline double getRand();

#endif