#ifndef __MAP_HPP__
#define __MAP_HPP__
#include <atomic>
#include <iterator>
#include <thread>
#include <vector>
#include "ant.hpp"

typedef struct {
    int x;
    int y;
} pos_t;

enum { EMPTY, FOOD, PHEROMONE };

class MapObj
{
public:
    MapObj()
    {
        this->food = 0;
        this->type = EMPTY;
    }
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
    // FIXME: need to atomic, prevent race condition
    vector<vector<MapObj>> vec;
    thread t;
    // Sync the map obj, create a thread to run it in background
    void sync();


public:
    // The pos is the width(x) and length(y)
    LocalMap(pos_t pos);
    ~LocalMap();

    // Check the the place have someting?
    MapObj &at(pos_t pos);

    // Merge the income obj, at the place
    // might be pheromone, food or empty
    void merge(pos_t pos, MapObj _source);
};

#endif