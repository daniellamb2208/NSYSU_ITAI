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
    l.put_at(pos_t(), MapObj(255, HOME));
    l.merge(pos_t(1, 2), MapObj(10, FOOD));
    l.merge(pos_t(1, 3), MapObj(255, PHEROMONE));
    l.merge(pos_t(1, 4), MapObj(0, EMPTY));
    print_map(l.show());

    // Put PHEROMONE on FOOD is FOOD
    l.merge(pos_t(1, 4), MapObj(40, FOOD));
    l.merge(pos_t(1, 4), MapObj(10, PHEROMONE));
    print_map(l.show());

    // Put FOOD on PHEROMONE is FOOD
    l.merge(pos_t(1, 5), MapObj(40, PHEROMONE));
    l.merge(pos_t(1, 5), MapObj(10, FOOD));
    print_map(l.show());

    // Put FOOD on HOME is HOME
    l.merge(pos_t(), MapObj(10, FOOD));
    print_map(l.show());

    // Put PHEROMONE on HOME is HOME
    l.merge(pos_t(), MapObj(40, PHEROMONE));
    print_map(l.show());

    return 0;
}
