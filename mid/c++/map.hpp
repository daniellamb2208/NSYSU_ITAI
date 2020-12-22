#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <array>
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#define HEIGHT 800  // test
#define WIDTH 600
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH)

const double DISCOUNT_LAMBDA = 1.1;
using namespace std;

enum { EMPTY, FOOD, PHEROMONE, HOME };

struct pos_t {
    size_t x, y;
    // Constructor
    pos_t() : x(0), y(0) {}
    pos_t(size_t _x, size_t _y) : x(_x), y(_y) {}
    pos_t(double _x, double _y) : x(long(_x)), y(long(_y)) {}
    pos_t(int _x, int _y) : x(_x), y(_y) {}
    pos_t(const pos_t &o) = default;

    friend double norm(const pos_t a);
    friend double cos(const pos_t a, const pos_t b);
    friend double dot(const pos_t a, const pos_t b);
    friend bool operator<(const pos_t a, const pos_t b);
    friend bool operator==(const pos_t a, const pos_t b);
    friend bool operator!=(const pos_t a, const pos_t b);

    pos_t &operator=(const pos_t src);
    pos_t operator+(const pos_t other);
    pos_t operator-(const pos_t other);
    pos_t operator*(const double times);
};


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
    MapObj operator+(const MapObj other);
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


#endif