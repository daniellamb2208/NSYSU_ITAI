#ifndef __INTERFACES_HPP__
#define __INTERFACES_HPP__

#include "../ant.hpp"
#include "../map.hpp"
using namespace std;

namespace ant_game
{
// Init this C++ backend, must be called first
void init();

// Returns vector of (type, pos_y, pos_x, value)
vector<tuple<int, int, int, double>> view();

// Let ants go
void go();

// Add one ant by default
bool add_ant(size_t num, pos_t pos);

// Put 100 food on pos_t
bool add_food(int x, int y);
}  // namespace ant_game

#endif