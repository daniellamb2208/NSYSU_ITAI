#include "ant.hpp"
#include <functional>
#include <memory>
#include <random>
using namespace std;
#define QUEEN_APPETITE 100
#define MALE_APPETITE 80
#define WORKER_APPETITE 1
#define SOLDIER_APPETITE 40

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

// Go to destination for single step
static inline void go(pos_t &curr, const pos_t &dest)
{
    long int x_diff = dest.x - curr.x;
    long int y_diff = dest.y - curr.y;
    // +1 prevent 0/0
    curr.x +=
        (abs(x_diff) > abs(y_diff)) ? ((x_diff | 1) / abs(x_diff | 1)) : 0;
    curr.y +=
        (abs(x_diff) <= abs(y_diff)) ? ((y_diff | 1) / abs(y_diff | 1)) : 0;

    // Overflow condition, move one step, so if overflow is occurred
    // previous position most be 0
    if (curr.x & (1 << 31))
        curr.x = 0;
    if (curr.y & (1 << 31))
        curr.y = 0;
    if (curr.x >= HEIGHT)
        curr.x = HEIGHT - 1;
    if (curr.y >= WIDTH)
        curr.y = WIDTH - 1;
}

static inline bool __alive_handler(Ant *me)
{
    // Check and set
    if (me->get_step() == 0 || me->get_energy() == 0)
        me->set_live_status(false);

    return me->get_live_status();
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
    this->job = move(my_job);
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
    // Each work comsume a step.
    me->set_step(me->get_step() - 1);
    // Get the closest of floor(i^3) form the opcode
    auto cube_convertion = [](int opcode) {
        // Loop over enum class from the end
        for (int i = static_cast<int>(Job_Code::End);
             i != static_cast<int>(Job_Code::queen); i--) {
            if (i * i * i < opcode)
                return i;
        }
        return 0;
    };

    // Pregnant process
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

    remove_dead_ant();
}

void Queen::alive_handler()
{
    if (!__alive_handler(me)) {
        // TODO: move my_slave to virgin
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
    // Add to my energy
    me->set_energy(me->get_energy() + QUEEN_APPETITE);
    return QUEEN_APPETITE;
}

void Queen::remove_dead_ant()
{
    for (auto iter = my_slave.begin(); iter != my_slave.end();) {
        if (!iter->get_live_status())
            iter = my_slave.erase(iter);
        else
            iter++;
    }
}

void Male::work()
{
    me->set_step(me->get_step() - 1);
    pos_t my_queen_pos = this->me->get_queen_pos();
    pos_t my_pos = this->me->at();

    auto entropy = get_rand_uniform() * 3 / 100.0;
    if (distance(my_pos, my_queen_pos) < entropy) {
        // Too close, leave more
        pos_t target(getRand() * HEIGHT, getRand() * WIDTH);
        go(this->me->at(), target);
    } else {
        go(this->me->at(), my_queen_pos);
    }
}

int Male::get_food()
{
    // TODO: Check have food at home
    // TODO: Eat the food, and Add it to my energy
    return MALE_APPETITE;
}

void Male::alive_handler()
{
    __alive_handler(me);
}

void Virgin::work()
{
    // Queen move her slave to me
    if (this->tmp_slave.size() && me->at() != new_home) {
        // TODO: Move to new house
    } else if (this->tmp_slave.size() && me->at() == new_home) {
        // TODO: Create a queen job, move slave to it, and delete this
        auto new_job = make_unique<Queen>(me);
        new_job->get_slave().swap(this->tmp_slave);  // Move ir
        me->set_job(move(new_job));                  // FIXME: might bug here
    } else {
        // Sleep, do nothing, not reduce my step
    }
}

void Virgin::alive_handler()
{
    __alive_handler(me);
}

void Worker::work()
{
    if (is_go_to_find_food && my_food.type == EMPTY)
        find_food();
    else if (my_food.type == EMPTY)
        my_food = pick_food();
    else
        return_home();
}

void Worker::put_pheromone(pos_t pos)
{
    me->get_map()->merge(pos, MapObj(100, PHEROMONE));
}

// Be careful! Here is eat the food from my hand
// Not pick up the food from env.
int Worker::get_food()
{
    if (my_food.type == FOOD) {
        my_food.value--;
        me->set_energy(me->get_energy() + WORKER_APPETITE);
    }
    return WORKER_APPETITE;
}

void Worker::alive_handler()
{
    __alive_handler(me);
}

void Worker::find_food()
{
    me->set_step(me->get_step() - 1);
    // TODO: go to find food, Use the dot product algorithm
    // TODO: If found food, set is_go_to_find_food to false
}

MapObj Worker::pick_food()
{
    auto my_map = me->get_map();
    // FIXME: have race condition here
    MapObj picked_up = my_map->get_at(me->at());
    my_map->merge(me->at(), MapObj(-picked_up.value, FOOD));
    return picked_up;
}

void Worker::return_home()
{
    me->set_step(me->get_step() - 1);
    if (my_food.value == 0) {
        // I had eaten the food when I take it back.
        my_food.type = EMPTY;
        is_go_to_find_food = true;
    }
    // TODO: Go home
}

void Soldier::work()
{
    me->set_step(me->get_step() - 1);
    pos_t q_pos = me->get_queen_pos();
    pos_t entropy(get_rand_uniform() / 49, get_rand_uniform() / 49);
    pos_t target = q_pos + entropy;
    go(me->at(), target);
}

int Soldier::get_food()
{
    // TODO: Check have food at home
    // TODO: Eat the food, and Add it to my energy
    // TODO: same as Male
    return SOLDIER_APPETITE;
}

void Soldier::alive_handler()
{
    __alive_handler(me);
}
