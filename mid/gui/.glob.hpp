
using namespace std;

// Parameters are modifiable
// Changing to observe the ant behaviour
#define STEP 1
#define CHILDREN_BASE 30
#define MAXSTEP 1500
#define MAXENERGY 1500
#define SLEEP_DURATION 0 // micro_seconds
#define PHEROMONE_FREQUENCY 1

enum class STATUS : bool
{
    DEAD,
    ALIVE,
};
struct pos_t;
class MapObj;
class LocalMap;

/**
 * All the ants are worker.
 *
 */
class Job;

class Ant
{
private:
    LocalMap *myMap = nullptr;
    // My position
    // Every steps should udpate this pos
    pos_t pos = pos_t();
    pos_t home_pos = pos_t();
    // Descent form threshold(MAXSTEP), will die if count to zero.
    int step = MAXSTEP;
    // Descenting energy, can be increased by eat food, but will die if
    // count to zero.
    int energy = MAXENERGY;
    STATUS is_alive = STATUS::ALIVE;

protected:
    friend class Job; // Make Job could access *this directly

public:
    unique_ptr<Job> job;

    Ant() = delete;
    Ant(LocalMap *_map, pos_t _my_home);
    Ant(const Ant &) = delete;
    Ant(Ant &&o)
        : myMap(o.myMap),
          pos(o.pos),
          home_pos(o.home_pos),
          step(o.step),
          energy(o.energy),
          is_alive(o.is_alive),
          job(move(o.job)){};
    Ant &operator=(const Ant &) = delete;
    Ant &operator=(Ant &&) = default;
    ~Ant() = default;

    pos_t &at() { return this->pos; }
    const pos_t &at() const { return this->pos; }
    void set_step(int _step) { this->step = _step; }
    const int get_step() const { return this->step; }
    void set_job(unique_ptr<Job> &&_job) { this->job = move(_job); }
    void set_map(LocalMap *_myMap) { this->myMap = _myMap; }
    LocalMap *get_map() const { return this->myMap; }
    int get_energy() const { return this->energy; }
    void set_energy(int _value) { this->energy = _value; }
    STATUS get_live_status() const { return this->is_alive; }
    void set_live_status(STATUS status) { this->is_alive = status; }
    pos_t &home() { return this->home_pos; }
    const pos_t &home() const { return this->home_pos; }
    const Ant *get_me() const { return this; };

    friend void info(const Ant *a);
};

class Job
{
protected:
    // Including walk, job, management
    virtual void work() = 0;
    // Eat the food which it get
    virtual void eat() = 0;
    virtual int get_food() = 0;
    // Set live status in each work being done
    virtual void alive_handler() = 0;
    virtual void clean() = 0;

public:
    virtual ~Job() {}
    Job() = default;
    Job(Job &&j) = default;
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
        alive_handler();
    }
    virtual void info() {}
};

class Worker : public Job
{
    bool is_go_to_find_food = true;
    MapObj my_food = MapObj(0, EMPTY);
    Ant *me;
    pos_t oriented;

    void work() final;
    void eat() final { me->set_energy(me->get_energy() - get_food()); }
    void put_pheromone(pos_t pos);
    int get_food() final;
    void alive_handler() final;
    void clean() final{};
    void find_food();
    MapObj pick_food();
    void put_food(pos_t pos);
    void return_home();

public:
    Worker(Worker &&w)
        : is_go_to_find_food(w.is_go_to_find_food),
          my_food(w.my_food),
          me(w.me),
          oriented(w.oriented)
    {
    }
    Worker(Ant *_me);
    ~Worker() { clean(); }
    void info() final;
};

#include <array>
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#define HEIGHT 10 // test
#define WIDTH 10
#define DISAPPEAR_THRESHOLD 0.3
#define MAX_FOOD (HEIGHT * WIDTH * 10)

const double DISCOUNT_LAMBDA = 1.1;
using namespace std;

enum
{
    EMPTY = 8,
    PHEROMONE = 4,
    FOOD = 2,
    HOME = 1
};

struct pos_t
{
    int x, y;
    // Constructor
    pos_t() : x(0), y(0) {}
    pos_t(size_t _x, size_t _y) : x(_x), y(_y) {}
    pos_t(double _x, double _y) : x(long(_x)), y(long(_y)) {}
    pos_t(int _x, int _y) : x(_x), y(_y) {}
    pos_t(const pos_t &o) = default;

    friend double norm(const pos_t a);
    friend double cos(const pos_t a, const pos_t b);
    friend double dot(const pos_t a, const pos_t b);
    friend bool operator<(const pos_t a, const pos_t b);
    friend bool operator==(const pos_t a, const pos_t b);
    friend bool operator!=(const pos_t a, const pos_t b);

    pos_t &operator=(const pos_t src);
    pos_t operator+(const pos_t other);
    pos_t operator+(const pos_t other) const;
    pos_t operator-(const pos_t other);
    pos_t operator-(const pos_t other) const;
    pos_t operator*(const double times);
    friend ostream &operator<<(ostream &_out, const pos_t pos);
};

class MapObj
{
public:
    MapObj() noexcept : value(0), type(EMPTY) {}
    MapObj(double _value, char _type);
    void clean()
    {
        this->type = EMPTY;
        this->value = 0;
    }
    union
    {
        double pheromone;
        double food;
        double value;
    };
    char type;
    MapObj operator+(MapObj other);
    MapObj &operator=(const MapObj &_source) = default;
};

class LocalMap
{
private:
    atomic<double> tot_foods;
    // This map is a static 2D array of atomic obj
    array<array<atomic<MapObj>, WIDTH>, HEIGHT> arr;
    thread t;
    // Sync this map obj, create a thread to run it in background
    void sync();

public:
    LocalMap() = delete;
    LocalMap(bool disable_sync);
    ~LocalMap();

    // Get the obj
    MapObj get_at(pos_t pos);
    // Put it forcibly
    void put_at(pos_t pos, MapObj obj);

    // Merge the income obj at that place
    // might be pheromone, food or empty
    // EMPTY + ANY_TYPE = ANY_TYPE
    void merge(pos_t pos, MapObj _source);

    // Generate some foods in random time, place, numbers and value by default
    // Guarantee no overcommit food(If require more then MAX, will be ignored)
    void food_gen(size_t num, double value);
    // Default call this function
    void food_gen();

    // Return 2D array of pair<double, char>, first is value, second is type
    array<array<pair<double, char>, WIDTH>, HEIGHT> show();
};

#include <functional>
#include <iostream>
#include <memory>
#include <random>
using namespace std;
#define WORKER_APPETITE 1

static const inline double std_normal()
{
    random_device rd;
    mt19937_64 gen = mt19937_64(rd());
    normal_distribution<double> dis(0, 1); // default [0, 100)
    return bind(dis, gen)();               // bind and call
}

// Go to destination for single step
static inline void go(pos_t &curr, const pos_t &dest)
{
    auto diff = dest - curr;
    // +1 prevent 0/0
    curr.x +=
        (abs(diff.x) > abs(diff.y)) ? ((diff.x | 1) / (abs(diff.x | 1))) : 0;
    curr.y +=
        (abs(diff.x) <= abs(diff.y)) ? ((diff.y | 1) / (abs(diff.y | 1))) : 0;

    // Overflow condition, move one step, so if overflow is occurred
    // previous position most be 0
    if (curr.x == -1)
        curr.x = HEIGHT - 1;
    if (curr.y == -1)
        curr.y = WIDTH - 1;
    if (curr.x >= HEIGHT)
        curr.x = 0;
    if (curr.y >= WIDTH)
        curr.y = 0;
}

namespace detail
{
    void walk(Ant *me, pos_t oriented)
    {
        auto &curr_pos = me->at();
        auto my_map = me->get_map();

        auto get_max_obj = [](vector<MapObj> &&v) {
            auto max_item = v[0];
            for (auto i : v)
                if (i.value > max_item.value)
                    max_item = i;
            return make_pair(pos_t(), max_item);
        };

        // Scan near by
        auto find_near = [&]() {
            MapObj a, b, c;

            // Might ou-of-range while get a, b, c
            a = my_map->get_at(pos_t((curr_pos.x + 1) % HEIGHT, curr_pos.y));
            b = my_map->get_at(
                pos_t((curr_pos.x + 1) % HEIGHT, (curr_pos.y + 1) % WIDTH));
            c = my_map->get_at(pos_t(curr_pos.x, (curr_pos.y + 1) % WIDTH));

            if (a.type == FOOD)
                return make_pair(pos_t((curr_pos.x + 1) % HEIGHT, curr_pos.y), a);
            else if (b.type == FOOD)
                return make_pair(
                    pos_t((curr_pos.x + 1) % HEIGHT, (curr_pos.y + 1) % WIDTH), b);
            else if (c.type == FOOD)
                return make_pair(pos_t(curr_pos.x, (curr_pos.y + 1) % WIDTH), c);
            else
                return get_max_obj({a, b, c});
        };

        //
        auto [near_where, near_what] = find_near();
        if (near_what.type == FOOD)
        {
            go(curr_pos, near_where);
        }
        else if (abs(std_normal()) < 1)
        {
            // follow other PHEROMONE
            go(curr_pos, near_where);
        }
        else
        {
            go(curr_pos, curr_pos + oriented);
        }
    }

}; // namespace detail

static inline STATUS __alive_handler(Ant *me)
{
    // Check and set
    if (me->get_step() == 0 || me->get_energy() == 0)
        me->set_live_status(STATUS::DEAD);

    return me->get_live_status();
}

Ant::Ant(LocalMap *_map, pos_t _my_home)
{
    this->job = make_unique<Worker>(this);
    this->myMap = _map;
    this->home_pos = _my_home;
}

void info(const Ant *a)
{
    // cerr << "In main: " << a << endl;
    cerr << "pos: \t(" << a->at().x << ", " << a->at().y << ")" << endl
         << "home: \t" << a->home() << endl
         << "step: \t" << a->get_step() << endl
         << "energy: " << a->get_energy() << endl
         << "alive: \t" << static_cast<bool>(a->get_live_status()) << endl;
    a->job->info();
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
    if (my_food.type == FOOD)
    {
        my_food.value--;
        me->set_energy(me->get_energy() + WORKER_APPETITE);
    }
    return WORKER_APPETITE;
}

void Worker::alive_handler()
{
    me->set_live_status(__alive_handler(me));
}

void Worker::find_food()
{
    // cerr << "In Ant: " << this->me << endl;
    me->set_step(me->get_step() - 1);
    auto my_map = me->get_map();
    auto curr_pos = me->at();
    // cerr << curr_pos << endl;
    if (my_map->get_at(curr_pos).type != FOOD)
        detail::walk(me, this->oriented);
    else // Next time will run `pick_food()`
        is_go_to_find_food = false;
}

MapObj Worker::pick_food()
{
    auto my_map = me->get_map();
    // FIXME: have race condition here
    MapObj picked_up = my_map->get_at(me->at());
    my_map->merge(me->at(), MapObj(-picked_up.value, FOOD));
    return picked_up;
}

void Worker::put_food(pos_t pos)
{
    auto my_map = me->get_map();
    my_map->merge(pos, my_food);
    my_food.clean();
}

void Worker::return_home()
{
    me->set_step(me->get_step() - 1);
    if (my_food.value <= 0)
    {
        // I had eaten the food when I take it back.
        my_food.clean();
        is_go_to_find_food = true;
    }
    put_pheromone(me->at());
    go(me->at(), me->home());

    if (me->at() == me->home())
    {
        put_food(me->home());
        is_go_to_find_food = true;
    }
}

Worker::Worker(Ant *_me = nullptr) : me(_me)
{
    pos_t __proto_oriented(0, 1);
    if (std_normal() > 0)
        swap(__proto_oriented.x, __proto_oriented.y);
    oriented = __proto_oriented;
}

void Worker::info()
{
    cerr << "go \t" << ((is_go_to_find_food) ? "food" : "home") << endl
         << "my \t" << my_food.type << " " << my_food.value << endl
         << "oriented (" << oriented.x << ", " << oriented.y << ")\n";
}

#include <array>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

// Separate `value` into `num` parts by normal distribution
static vector<double> __sep_norm(size_t num, double value)
{
    vector<double> result;
    result.resize(num);
    // TODO: separate `value` into `num` parts by normal distribution
    for (size_t i = 0; i < num; i++)
        result.at(i) = value / num; // Use uniform currently
    return result;
}

// Return [0,1) double
static inline double __get_rand()
{
    random_device rd;
    mt19937 gen = mt19937(rd());
    uniform_real_distribution<> dis(0, 1); // return double
    return bind(dis, gen)();
}

double norm(const pos_t a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

double cos(const pos_t a, const pos_t b)
{
    return dot(a, b) / (norm(a) * norm(b));
}

double dot(const pos_t a, const pos_t b)
{
    return a.x * b.x + a.y * b.y;
}

bool operator<(const pos_t a, const pos_t b)
{
    return norm(a) < norm(b);
}

bool operator==(const pos_t a, const pos_t b)
{
    return (a.x == b.x) && (a.y == b.y);
}

bool operator!=(const pos_t a, const pos_t b)
{
    return !(a == b);
}

pos_t &pos_t::operator=(const pos_t src)
{
    memcpy(this, &src, sizeof(src));
    return *this;
}

pos_t pos_t::operator+(const pos_t other)
{
    this->x += other.x;
    this->y += other.y;
    return *this;
}

pos_t pos_t::operator+(const pos_t other) const
{
    return pos_t(x + other.x, y + other.y);
}

pos_t pos_t::operator-(const pos_t other)
{
    this->x -= other.x;
    this->y -= other.y;
    return *this;
}

pos_t pos_t::operator-(const pos_t other) const
{
    return pos_t(x - other.x, y - other.y);
}

pos_t pos_t::operator*(double times)
{
    this->x *= times;
    this->y *= times;
    return *this;
}
ostream &operator<<(ostream &_out, const pos_t pos)
{
    _out << "(" << pos.x << ", " << pos.y << ") ";
    return _out;
}

MapObj::MapObj(double _value, char _type) : value(_value), type(_type) {}

MapObj MapObj::operator+(MapObj other)
{
    auto ffs = __builtin_ffs(this->type | other.type);
    auto merge_type = 1 << (ffs - 1);

    auto partner_ptr = (merge_type == other.type) ? this : (&other);
    auto pivot_ptr = (partner_ptr == this) ? (&other) : this;
    switch (merge_type)
    {
    case HOME:
    case FOOD:
        this->value = pivot_ptr->value + (((partner_ptr->type == FOOD) ||
                                           (partner_ptr->type == HOME))
                                              ? partner_ptr->value
                                              : 0);
        break;
    case PHEROMONE:
        this->value = pivot_ptr->value + partner_ptr->value;
        break;
    case EMPTY:
    default:
        break;
    }
    this->type = merge_type;
    if (this->value < 0)
        this->clean();
    return *this;
}

void LocalMap::sync()
{
    auto proc_unit = [&](MapObj obj, auto &jter) {
        auto preValue = obj.value;
        if (obj.value < 0.03 && obj.type != HOME)
            obj.clean();

        // PHEROMONE will dissipate
        if (obj.type == PHEROMONE)
            obj.value /= DISCOUNT_LAMBDA;

        // Update the array
        jter->store(obj);

        // Update tot_foods
        if (obj.type == FOOD)
        {
            auto orig_tot_foods = tot_foods.load();
            tot_foods.store(orig_tot_foods + obj.value - preValue);
        }
    };

    while (true)
    {
        for (auto iter = arr.begin(); iter != arr.end(); iter++)
            for (auto jter = iter->begin(); jter != iter->end(); jter++)
                proc_unit(jter->load(), jter);

        food_gen();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

LocalMap::LocalMap(bool disable_sync = false) : tot_foods(0)
{
    // Initialize arr
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++)
            arr.at(i).at(j) = MapObj();

    if (!disable_sync)
        this->t = thread(bind(&LocalMap::sync, this));
}

LocalMap::~LocalMap()
{
    if (t.joinable())
        this->t.join();
}

MapObj LocalMap::get_at(pos_t pos)
{
    MapObj obj;
    try
    {
        obj = this->arr.at(pos.x).at(pos.y).load();
    }
    catch (const std::exception &e)
    {
        throw e;
    }
    return obj;
}

void LocalMap::put_at(pos_t pos, MapObj obj)
{
    double preValue = get_at(pos).value;
    (this->arr.at(pos.x).at(pos.y)).store(obj);

    if (obj.type == FOOD)
    {
        double postValue = get_at(pos).value;
        double orig_tot_foods = this->tot_foods.load();
        this->tot_foods.store(orig_tot_foods + (postValue - preValue));
    }
}

void LocalMap::merge(pos_t pos, MapObj _source)
{
    auto me = this->get_at(pos);
    me = me + _source;
    this->put_at(pos, me);
}

// Default callee
void LocalMap::food_gen()
{
    auto value = int(__get_rand() * MAX_FOOD);

    size_t num = __get_rand() * 50; // 50 food? I guess.
    food_gen(num, value);
}

// Separeate foods normally
void LocalMap::food_gen(size_t num, double value)
{
    if (value + this->tot_foods.load() > MAX_FOOD)
        return;
    auto values = __sep_norm(num, value);

    // Because each `pos` obj is independent,
    // and each obj might have blocking IO from atomic operation
    // TODO: Parallelize it!
    for (auto i : values)
    {
        pos_t pos(size_t(__get_rand() * HEIGHT), size_t(__get_rand() * WIDTH));
        this->merge(pos, MapObj(i, FOOD));
    }
}

array<array<pair<double, char>, WIDTH>, HEIGHT> LocalMap::show()
{
    array<array<pair<double, char>, WIDTH>, HEIGHT> canvax;
    for (size_t i = 0; i < HEIGHT; i++)
        for (size_t j = 0; j < WIDTH; j++)
        {
            auto obj = get_at(pos_t(i, j));
            canvax.at(i).at(j) = make_pair(obj.value, obj.type);
        }
    // Return 2-dimension array of double
    // You should cast the type to char by yourself(if you want type)
    return canvax;
}

namespace ant_game
{
    // Init this C++ backend, must be called first
    void init();

    // Returns vector of (type, pos_x, pos_y, value)
    vector<tuple<int, int, int, double>> view();

    // Add one ant by default
    bool add_ant(size_t num);

    // Put 100 food on pos_t
    bool add_food(int x, int y);
} // namespace ant_game

#include <tuple>
#include <utility>
#include <vector>
using namespace std;

namespace
{
    vector<unique_ptr<Ant>> ant_pool;
    LocalMap localMap(false);
} // namespace

void ant_game::init()
{
    localMap.put_at(pos_t(), MapObj(0, HOME));
    ant_pool.push_back(make_unique<Ant>(&localMap, pos_t(0, 0)));
}

vector<tuple<int, int, int, double>> ant_game::view()
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

bool ant_game::add_ant(size_t num = 1)
{
    try
    {
        for (size_t i = 0; i < num; i++)
            ant_pool.push_back(make_unique<Ant>(&localMap, pos_t(0, 0)));
    }
    catch (const exception &e)
    {
        throw e;
        return false;
    }
    return true;
}

bool ant_game::add_food(int x, int y)
{
    pos_t _pos(x, y);
    try
    {
        localMap.merge(_pos, MapObj(100, FOOD));
    }
    catch (const exception &e)
    {
        throw e;
        return false;
    }
    return true;
}

using namespace std;

namespace ant_game
{
    // Init this C++ backend, must be called first
    void init();

    // Returns vector of (type, pos_x, pos_y, value)
    vector<tuple<int, int, int, double>> view();

    // Add one ant by default
    bool add_ant(size_t num);

    // Put 100 food on pos_t
    bool add_food(int x, int y);
} // namespace ant_game
