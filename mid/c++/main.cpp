#include <unistd.h>
#include <iostream>
#include "ant.hpp"
#include "map.hpp"
using namespace std;

void printMap(LocalMap *localMap, int p)
{
    auto cast = [](int _p, double value) {
        if (_p)
            cout << (value) << " ";
        else
            cout << char(value) << " ";
    };
    auto m = localMap->show(p);
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++)
            cast(p, m[i][j]);
        cout << endl;
    }
    cout << endl;
}

int main()
{
    LocalMap localMap;
    localMap.put_at(pos_t(0, 0), MapObj(1 << 30, HOME));

    vector<Ant> ant_pool;
    for (int i = 0; i < 1; i++)
        ant_pool.push_back(move(Ant(&localMap, pos_t(0, 0))));
    while (!ant_pool.empty()) {
        for (auto &i : ant_pool)
            i.job->do_job();

        // Collect dead ant
        for (auto iter = ant_pool.begin(); iter != ant_pool.end();) {
            if (iter->get_live_status() == STATUS::DEAD)
                iter = ant_pool.erase(iter);
            else
                iter++;
        }
        printMap(&localMap, 1);
        printMap(&localMap, 0);
    }

    return 0;
}

/**
 *
 * Many many ants
 *                         -----------
 *                        |    Ant    |    X   N
 *                         -----------
 *                         /         \
 *                        /           \
 *                       /             \
 *                 -----------         -----------
 *                |   Breed   |       | Non-breed |
 *                 -----------         -----------
 *               /           \            /      \
 *           ------       -------      -------    ------
 *          | Male |   →→| Queen | ←  |Soldier|  |Worker|
 *           ------    ↑  -------  ↑   -------    -------
 *              ↓→→→→→→→     |     ↑------↓         ↓
 *                 sex    --------     protect      ↓
 *                       | Virgin |                 ↓
 *                        --------                  ↓     pick/put some things
 *                           ↓                      ↓
 *                       pregnant                   ↓
 *-------------------ground----------------------------------
 *
 *  --------
 * |  Sync  | - Synchronize the ground.
 *  --------        Self-management, Just like the Earth will balance the
 *                  environment autometically.
 *
 *  --------
 * |  merge  | - Merge objects(You pick/put something on the ground).
 *  --------        Like a system call, kernel will help you to handle it.
 *
 *
 *
 *  -----------------
 * |  food generator | - Earth will give animals food to live.
 *  -----------------
 *
 *
 */