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


void ant_game::init()
{
    localMap.put_at(pos_t(), MapObj(0, HOME));
    for (int i = 0; i < 5; i++)
        add_ant(1, pos_t());
    // ant_pool.push_back(make_unique<Ant>(&localMap, pos_t(0, 0)));
}

vector<tuple<int, int, int, double>> ant_game::view()
{
    vector<tuple<int, int, int, double>> v;

    auto map_print = localMap.show();
    for (size_t i = 0; i != map_print.size(); i++)
        for (size_t j = 0; j != map_print.front().size(); j++)
            if (map_print[i][j].second != EMPTY)
                v.push_back(make_tuple(map_print[i][j].second, j * 10, i * 10,
                                       map_print[i][j].first));


    // Type of Ant is 0
    for (auto &i : ant_pool)
        v.push_back(make_tuple(0, i->at().y * 10, i->at().x * 10,
                               ((i->get_step() + i->get_energy()) * 100.0 /
                                (MAXSTEP + MAXENERGY))));
    return v;
}

void ant_game::go()
{
    for (auto &i : ant_pool) {
        i->job->do_job();
        fo(i->get_me());
    }
    for (auto iter = ant_pool.begin(); iter != ant_pool.end();) {
        if (iter->get()->get_live_status() == STATUS::DEAD)
            iter = ant_pool.erase(iter);
        else
            iter++;
    }
}

void ant_game::birth()
{
    auto num = localMap.get_at(pos_t()).value / 200;
    localMap.merge(pos_t(), MapObj((-1) * 50 * num, HOME));
    add_ant(num, pos_t());
}

bool ant_game::add_ant(size_t num, pos_t pos = pos_t(0, 0))
{
    try {
        for (size_t i = 0; i < num; i++)
            ant_pool.push_back(make_unique<Ant>(&localMap, pos));
    } catch (const exception &e) {
        throw e;
        return false;
    }
    return true;
}

bool ant_game::add_food(int x, int y)
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