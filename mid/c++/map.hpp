#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <array>
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#define HEIGHT 60  // test
#define WIDTH 80
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH)

const double DISCOUNT_LAMBDA = 1.01;
using namespace std;

enum { EMPTY = 8, PHEROMONE = 4, FOOD = 2, HOME = 1 };

struct pos_t {
    int x, y;
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
    pos_t operator+(const pos_t other) const;
    pos_t operator-(const pos_t other);
    pos_t operator-(const pos_t other) const;
    pos_t operator*(const double times);
    friend ostream &operator<<(ostream &_out, const pos_t pos);
};


class MapObj
{
public:
    MapObj() noexcept : value(0), type(EMPTY) {}
    MapObj(double _value, char _type);
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
    MapObj operator+(MapObj other);
    MapObj &operator=(const MapObj &_source) = default;
};

class LocalMap
{
private:
    atomic<double> tot_foods;
    // This map is a static 2D array of atomic obj
    array<array<atomic<MapObj>, WIDTH>, HEIGHT> arr;
    thread t;
    // Sync this map obj, create a thread to run it in background
    void sync();

public:
    LocalMap() = delete;
    LocalMap(bool disable_sync);
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
    void food_gen(size_t num, double value);
    // Default call this function
    void food_gen();

    // Return 2D array of pair<double, char>, first is value, second is type
    array<array<pair<double, char>, WIDTH>, HEIGHT> show();
};


#endif