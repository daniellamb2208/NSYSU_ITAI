#include <unistd.h>
#include <iostream>
#include "ant.hpp"
#include "map.hpp"
using namespace std;

void printMap(LocalMap *localMap)
{
    auto m = localMap->show();
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++)
            cout << char(m[i][j]) << " ";
        cout << endl;
    }
    cout << endl;
}

int main()
{
    LocalMap localMap;
    localMap.foodGenerator();
    localMap.put_at(pos_t(0, 0), MapObj(50));

    vector<shared_ptr<Queen>> queenPtrPool;
    shared_ptr<Queen> initQueen =
        make_shared<Queen>(&localMap, pos_t(0, 0), &queenPtrPool);
    queenPtrPool.push_back(initQueen);

    while (!queenPtrPool.empty()) {  // still have ants
        for (auto &i : queenPtrPool) {
            for (auto &j : i->getSlave()) {
                j.get().job();  // slave job
            }
            i.get()->job();  // queen or Virgin job
        }
        printMap(&localMap);
        sleep(1);
    }
    return 0;
}