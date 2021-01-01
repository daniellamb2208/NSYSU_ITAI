#include <unistd.h>
#include <iostream>
#include "../ant.hpp"
#include "../map.hpp"
#include "color.hpp"
using namespace std;

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
    LocalMap localMap(false);
    localMap.food_gen();
    localMap.put_at(pos_t(0, 0), MapObj(0, HOME));

    vector<Ant> ant_pool;
    for (int i = 0; i < 1; i++)
        ant_pool.push_back(move(Ant(&localMap, pos_t(0, 0))));

    while (!ant_pool.empty()) {
        for (auto &i : ant_pool) {
            i.job->do_job();
            info(&i);
        }

        // Collect dead ant
        for (auto iter = ant_pool.begin(); iter != ant_pool.end();) {
            if (iter->get_live_status() == STATUS::DEAD)
                iter = ant_pool.erase(iter);
            else
                iter++;
        }
        print_map(localMap.show());
        sleep(1);
    }

    return 0;
}