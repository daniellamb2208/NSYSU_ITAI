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
    localMap.foodGenerator();
    localMap.put_at(pos_t(0, 0), MapObj(1 << 30, HOME));

    vector<shared_ptr<Queen>> queenPtrPool = {
        make_shared<Queen>(&localMap, pos_t(0, 0), &queenPtrPool)};

    while (!queenPtrPool.empty()) {  // still have ants
        for (auto &i : queenPtrPool) {
            i.get()->job();  // queen or Virgin job
            if (&i->getSlave())
                for (auto j : i->getSlave())
                    j->job();  // slave job
            // cout << i->getSlave().size() << endl;
        }
        printMap(&localMap, 1);
        printMap(&localMap, 0);
        // sleep(1);
    }
    return 0;
}