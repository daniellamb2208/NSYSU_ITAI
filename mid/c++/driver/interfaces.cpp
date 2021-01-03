#include "interfaces.hpp"
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

namespace
{
vector<unique_ptr<Ant>> ant_pool;
LocalMap localMap(false);
}  // namespace

namespace ant_game
{
void init()
{
    localMap.put_at(pos_t(), MapObj(0, HOME));
    ant_pool.push_back(make_unique<Ant>(&localMap, pos_t(0, 0)));
}

vector<tuple<int, int, int, double>> view()
{
    vector<tuple<int, int, int, double>> v;

    auto map_print = localMap.show();
    for (size_t i = 0; i != map_print.size(); i++)
        for (size_t j = 0; j != map_print.front().size(); j++)
            if (map_print[i][j].second != EMPTY)
                v.push_back(make_tuple(map_print[i][j].second, i, j,
                                       map_print[i][j].first));


    // Type of Ant is 0
    for (auto &i : ant_pool)
        v.push_back(make_tuple(0, i->at().x, i->at().y,
                               ((i->get_step() + i->get_energy()) * 100.0 /
                                (MAXSTEP + MAXENERGY))));
    return v;
}

bool add_ant(size_t num = 1)
{
    try {
        for (size_t i = 0; i < num; i++)
            ant_pool.push_back(make_unique<Ant>(&localMap, pos_t(0, 0)));
    } catch (const exception &e) {
        throw e;
        return false;
    }
    return true;
}

bool add_food(int x, int y)
{
    pos_t _pos(x, y);
    try {
        localMap.merge(_pos, MapObj(100, FOOD));
    } catch (const exception &e) {
        throw e;
        return false;
    }
    return true;
}
}  // namespace ant_game