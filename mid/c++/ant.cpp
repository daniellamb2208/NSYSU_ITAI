#include "ant.hpp"
#include <functional>
#include <random>
using namespace std;
#define QUEEN_APPETITE 100

static inline const int get_rand_uniform(int cube_sum_range = 5) noexcept
{
    unsigned long long bound = 0;
    for (int i = 0; i < cube_sum_range; i++)
        bound += i * i * i;

    random_device rd;
    mt19937_64 gen = mt19937_64(rd());
    uniform_int_distribution<int> dis(0, bound);  // default [0, 100)
    return bind(dis, gen)();                      // bind and call
}

Ant::Ant(int job_code)
{
    unique_ptr<Job> my_job = nullptr;
    switch (static_cast<Job_Code>(job_code)) {
    case Job_Code::queen:
        my_job = make_unique<Queen>(this);
        break;
    case Job_Code::virgin:
        my_job = make_unique<Virgin>(this);
        break;
    case Job_Code::male:
        my_job = make_unique<Male>(this);
        break;
    case Job_Code::solder:
        my_job = make_unique<Soldier>(this);
        break;
    case Job_Code::worker:
        my_job = make_unique<Worker>(this);
        break;
    default:
        throw runtime_error("Wrong opcode");
    }
    this->job = my_job.get();
    this->my_job_code = static_cast<Job_Code>(job_code);
}


// Any Organism has a vocation by the God
// The ant would have the selected vocation.
void Queen::pregnant(Job_Code _gift)
{
    // TODO: Create new ant with job, and push it into `my_slave`
}

void Queen::work()
{
    // Get the closest of floor(i^3) form the opcode
    auto cube_convertion = [](int opcode) {
        // Loop over enum class from the end
        for (int i = static_cast<int>(Job_Code::End);
             i != static_cast<int>(Job_Code::queen); i--) {
            if (i * i * i < opcode)
                return i;
        }
    };

    if (bleed()) {
        // The probability of Job_code is:
        // queen : 0%
        // virgin: 1%
        // male  : 4%
        // solder: 27%
        // worker: 64%
        int opcode =
            cube_convertion(get_rand_uniform(static_cast<int>(Job_Code::End)));
        pregnant(static_cast<Job_Code>(opcode));
    } else {
        pregnant(Job_Code::male);
    }
}

bool Queen::find_male()
{
    pos_t my_pos = me->at();
    // TODO: More efficiency
    for (auto iter = my_slave.begin(); iter != my_slave.end(); iter++) {
        // If is found:
        if (iter->get_job_code() == Job_Code::male && iter->at() == my_pos) {
            my_slave.erase(iter);
            return true;
        }
    }
    // If is not found
    return false;
}

int Queen::get_food()
{
    LocalMap *my_map = me->get_map();
    // TODO: Is it enough food to take from home?  Check it!
    MapObj consumption = MapObj(-QUEEN_APPETITE);
    my_map->merge(me->at(), consumption);
    return 100;
}

void Male::work()
{
    // TODO: Walk around my_queen
    pos_t my_queen_pos = this->me->get_queen_pos();
    pos_t my_pos = this->me->at();
}

