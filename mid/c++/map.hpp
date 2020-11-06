#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <array>
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#define HEIGHT 600
#define WIDTH 800
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH)

const int DISCOUNT_LAMBDA = 1.1;
using namespace std;

enum { EMPTY, FOOD, PHEROMONE, HOME };

typedef struct _p {
    int x;
    int y;
    // Length of distance
    friend double operator-(const struct _p(&a), const struct _p(&b))
    {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }
    friend bool operator<(const struct _p(&a), const struct _p(&b))
    {
        return (a.x * a.x) + (a.y * a.y) < (b.x * b.x) + (b.y * b.y);
    }
} pos_t;

class MapObj
{
public:
    MapObj()
    {
        this->value = 0;
        this->type = EMPTY;
    }
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
    static atomic<double> totalFoods;
    // This map is a static 2D array of atomic obj
    static array<array<atomic<MapObj>, WIDTH>, HEIGHT> arr;
    // This is a naive way to find the closest obj
    // Store all PHEROMONE and FOOD's position to this obj pool
    // And brute force linear searching all obj
    // access by objPool[FOOD][pos];
    map<char, map<pos_t, MapObj>> objPool;
    thread t;
    // Sync this map obj, create a thread to run it in background
    static void sync();

public:
    LocalMap();
    ~LocalMap();

    // Check the the place have someting?
    MapObj get_at(pos_t pos);
    // Put it forcibly
    void put_at(pos_t pos, MapObj thing);

    // Merge the income obj at that place
    // might be pheromone, food or empty
    // EMPTY + ANY_TYPE = ANY_TYPE
    void merge(pos_t pos, MapObj _source);


    // Generate some foods in random time, place, numbers and value by default
    // Guarantee no overcommit food(If require more then MAX, will be ignored)
    friend void foodGenerator(pos_t _pos,
                              size_t num,
                              double value,
                              LocalMap &_map);
    // Default call this function
    friend void foodGenerator(LocalMap &_map);

    // TODO: find some object is closest in the `type`
    // There is some algorithm called "Space Partitioning"
    // https://stackoverflow.com/questions/28154450/find-nearest-object-in-2d-can-it-be-optimised-below-on/28155167#28155167
    pos_t findClosest(const pos_t &currentPos, char type);
};

// Must be static member function
void LocalMap::sync()
{
    // i for first loop, j for second loop,
    // iterator is the operand named [ij]ter
    for (auto iter = arr.begin(); iter != arr.end(); iter++)
        for (auto jter = iter->begin(); jter != iter->end(); jter++) {
            auto obj = jter->load();
            if (obj.type != EMPTY) {
                auto preValue = obj.value;
                if (obj.value < 0.03)
                    obj.clean();
                // PHEROMONE will dissipate
                if (obj.type == PHEROMONE) {
                    auto me = obj;
                    me.value /= DISCOUNT_LAMBDA;
                    jter->store(me);
                }
                // Update totalFoods
                auto origTotalFoods = totalFoods.load();
                totalFoods.store(origTotalFoods + jter->load().value -
                                 preValue);
            }  // sparse array skip the EMPTY directly
        }
}

#endif