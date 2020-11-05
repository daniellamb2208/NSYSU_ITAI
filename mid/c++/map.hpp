#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <array>
#include <atomic>
#include <iterator>
#include <thread>
#define HEIGHT 600
#define WIDTH 800
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH)

const int DISCOUNT_LAMBDA = 1.1;
using namespace std;

typedef struct {
    int x;
    int y;
} pos_t;

enum { EMPTY, FOOD, PHEROMONE, HOME };

class MapObj
{
public:
    MapObj()
    {
        this->food = 0;
        this->type = EMPTY;
    }
    MapObj(double _value) : value(_value), type(FOOD) {}
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
    // This map is a static 2D array of atomic obj
    static array<array<atomic<MapObj>, WIDTH>, HEIGHT> arr;
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
};

// Must be static member function
void LocalMap::sync()
{
    // i for first loop, j for second loop,
    // iterator is the operand named [ij]ter
    for (auto iter = arr.begin(); iter != arr.end(); iter++)
        for (auto jter = iter->begin(); jter != iter->end(); jter++) {
            if (jter->load().type != EMPTY && jter->load().value < 0.03)
                jter->load().clean();
            // PHEROMONE will dissipate
            if (jter->load().type == PHEROMONE) {
                auto me = jter->load();
                me.value /= DISCOUNT_LAMBDA;
                jter->store(me);
            }
        }
}

// TODO: find some object is closest in the `type`
// There is some algorithm called "Space Partitioning"
// https://stackoverflow.com/questions/28154450/find-nearest-object-in-2d-can-it-be-optimised-below-on/28155167#28155167
pos_t findClosest(pos_t currentPos, int type);


#endif