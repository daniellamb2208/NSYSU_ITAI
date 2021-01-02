#include <unistd.h>
#include <array>
#include <iostream>
#include "../map.hpp"
#include "color.hpp"

void print_map(auto map)
{
    auto color_mux = [](char type) {
        if (type == FOOD)
            cout << Color::blue;
        else if (type == PHEROMONE)
            cout << Color::green;
        else if (type == HOME)
            cout << Color::yellow;
    };
    for (auto i : map) {
        for (auto j : i) {
            color_mux(j.second);
            std::cout << (j.first) << Color::default_c << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
    LocalMap l(true);
    // EMPTY + HOME = HOME
    l.merge(pos_t(), MapObj(10, HOME));
    // EMPTY + FOOD = FOOD
    l.merge(pos_t(1, 2), MapObj(20, FOOD));
    // EMPTY + PHEROMONE = PHEROMONE
    l.merge(pos_t(1, 3), MapObj(40, PHEROMONE));
    // EMPTY + EMPTY = EMPTY
    l.merge(pos_t(1, 4), MapObj(0, EMPTY));
    print_map(l.show());



    // FOOD + PHEROMONE = FOOD
    l.merge(pos_t(1, 4), MapObj(20, FOOD));
    l.merge(pos_t(1, 4), MapObj(40, PHEROMONE));
    print_map(l.show());

    // PHEROMONE + FOOD = FOOD
    l.merge(pos_t(1, 5), MapObj(40, PHEROMONE));
    l.merge(pos_t(1, 5), MapObj(20, FOOD));
    print_map(l.show());

    // FOOD + HOME = HOME
    l.merge(pos_t(), MapObj(20, FOOD));
    print_map(l.show());

    // PHEROMONE + HOME = no change
    l.merge(pos_t(), MapObj(40, PHEROMONE));
    print_map(l.show());

    // FOOD + HOME = HOME
    l.merge(pos_t(1, 5), MapObj(10, HOME));
    print_map(l.show());

    return 0;
}
