#include "ant.hpp"
#include <math.h>
#include <unistd.h>
#include <cmath>
#include <iostream>
#include <utility>
using namespace std;

// Go to destination for single step
static inline void go(pos_t &curr, const pos_t &dest)
{
    long long int x_diff = dest.x - curr.x;
    long long int y_diff = dest.y - curr.y;
    // +1 prevent 0/0
    curr.x += (abs(x_diff) > abs(y_diff)) ? ((x_diff + 1) / abs(x_diff)) : 0;
    curr.y += (abs(x_diff) <= abs(y_diff)) ? ((y_diff + 1) / abs(y_diff)) : 0;
    // Overflow condition, move one step, so if overflow is occurred previous
    // position most be 0
    if (curr.x & (1 << 31))
        curr.x = 0;
    if (curr.y & (1 << 31))
        curr.y = 0;
}

template <typename Base, typename T>
inline bool instanceof (const T &)
{
    return std::is_base_of<Base, T>::value;
}

Ant::~Ant()
{
    // Put foods down
    if (this->myObj.value > 0)
        myMap->merge(this->pos, this->myObj);
}

void Ant::setStep(int _step)
{
    this->step = _step;
}

const int Ant::getStep()
{
    return this->step;
}

void Ant::die()
{
    // compiler make me do this
    // origin is this->!Ant();
    // by lamb
    this->Ant::~Ant();
}

void Ant::sync() {}

void Ant::setMap(LocalMap *_myMap)
{
    this->myMap = _myMap;
}

pos_t &Ant::at()
{
    return this->pos;
}

LocalMap *Ant::getMap()
{
    return this->myMap;
}
MapObj Ant::getObj()
{
    return this->myObj;
}

void Ant::setMyQueen(Queen *_myQueen)
{
    this->myQueen = _myQueen;
}

pos_t Ant::getQueenPos()
{
    return this->myQueen->at();
}


// ----------------

BreedAnt::BreedAnt(LocalMap *_myMap, double _appetite)
{
    this->appetite = _appetite;
    this->myMap = _myMap;
}

BreedAnt::~BreedAnt() {}

void BreedAnt::setAppetite(double _appetite)
{
    this->appetite = _appetite;
}

double BreedAnt::getAppetite()
{
    return this->appetite;
}

void BreedAnt::eat()
{
    auto obj = this->myMap->get_at(this->pos);
    usleep(SLEEP_DURATION);
    obj.value -= this->appetite;
    this->myMap->put_at(this->pos, obj);
}

NonBreedAnt::NonBreedAnt(LocalMap *_myMap)
{
    myMap = _myMap;
}

NonBreedAnt::~NonBreedAnt() {}
void NonBreedAnt::take(MapObj &obj, pos_t dest)
{
    // Pick it up
    this->myObj = obj;
    this->myMap->put_at(this->pos, MapObj());  // put EMPTY obj

    // Goto dest
    while (this->pos != dest) {
        go(this->pos, dest);
        usleep(SLEEP_DURATION);
    }

    // Put it down
    this->myMap->merge(dest, this->myObj);
    this->myObj.clean();
}

// ----------------

Queen::Queen(LocalMap *_myMap,
             pos_t _pos,
             vector<shared_ptr<Queen>> *_queenPtrPool)
{
    myMap = _myMap;
    pos = _pos;
    this->queenPtrPool = _queenPtrPool;
    std::cout << "hi, I am queen" << endl;
}

Queen::~Queen() {}

int Queen::pregnant()
{
    // Number of children
    int num = int(getRand() * 10 + CHILDREN_BASE);
    auto setAttr = [this](Ant &prototype) {
        prototype.setMap(myMap);
        prototype.at() = pos;
        prototype.setStep(MAXSTEP);
    };

    for (int i = 0; i < num && (myMap->get_at(pos).value > 0); i++) {
        this->eat();

        // 10% Virgin, 20% Male, 30% Soldier, 40% Worker
        auto selectIndex = int(floor(getRand() * 10));
        if (selectIndex < 2) {  // Virgin
            Virgin prototype;
            setAttr(prototype);

            prototype.setQPP(this->queenPtrPool);
            prototype.setAppetite(2);
            auto partial = this->myMap->get_at(this->pos);
            partial.value /= 20;  // pick up some orig value
            prototype.myObj = partial;
            prototype.pos = pos_t(getRand() * HEIGHT, getRand() * WIDTH);
            prototype.slave = this->slave;
            // FIXME: Might SEGFAULT?
            // FIXME: push in will launch?
            prototype.queenPtrPool->push_back(make_shared<Virgin>(prototype));
            continue;
        } else if (selectIndex < 4) {  // Male
            Male prototype;
            setAttr(prototype);
            prototype.setAppetite(0.7);
            this->slave.push_back(prototype);
        } else if (selectIndex < 7) {  // Soldier
            Soldier prototype;
            setAttr(prototype);
            prototype.setMyQueen(this);
            prototype.setProtectDistance(10);
            this->slave.push_back(prototype);
        } else {  // Worker
            Worker prototype;
            setAttr(prototype);
            prototype.setMyQueen(this);
            this->slave.push_back(prototype);
        }
        usleep(SLEEP_DURATION);
    }
    return num;
}

vector<reference_wrapper<Ant>> &Queen::getSlave()
{
    return this->slave;
}

// Thoughts: food centrally controlled by Queen ant
void Queen::job()
{
    if (this->myMap->get_at(this->pos).value > 0) {
        this->eat();
        if (findMale())
            pregnant();
        else {  // If not found Male, go sleep and make a `Male`
            usleep(SLEEP_DURATION);
            Male prototype;
            prototype.setMap(myMap);
            prototype.at() = pos;
            prototype.setStep(MAXSTEP);
            prototype.setAppetite(0.7);
            this->slave.push_back(prototype);
        }
    }
}

inline const bool Queen::findMale()
{
    // Sensor if any Male ant on itself(position overlap)
    // if sensored, mate with it, and Male ant will die
    for (auto iter = this->slave.begin(); iter != this->slave.begin(); iter++) {
        if (instanceof <Male>(iter->get()) && iter->get().at() == this->pos) {
            // erase it will be destructed automatically
            this->slave.erase(iter);
            return true;
        }
    }

    return false;
}

void Queen::setQPP(vector<shared_ptr<Queen>> *_queenPtrPool)
{
    this->queenPtrPool = _queenPtrPool;
}

Male::Male(LocalMap *_myMap)
{
    myMap = _myMap;
    std::cout << "hi, I am Male" << endl;
}

Male::~Male() {}

void Male::setAppetite(double _appetite)
{
    this->appetite = _appetite;
}

void Male::job()
{
    if (this->step--) {
        // Hanging around Queen ant
        auto origPos = getQueenPos();
        auto entropy = getRand() * 5;
        if ((this->pos - origPos) < entropy) {
            // Too close, leave more
            pos_t target(getRand() * HEIGHT, getRand() * WIDTH);
            go(this->pos, target);
            eat();
        } else
            go(this->pos, origPos);  // closer
    } else
        this->die();
}

Virgin::Virgin(LocalMap *_myMap)
{
    myMap = _myMap;
    this->myMap->merge(this->pos, this->myObj);
    std::cout << "hi, I am virgin" << endl;
}

Virgin::~Virgin() {}

Soldier::Soldier(LocalMap *_myMap)
{
    myMap = _myMap;
    std::cout << "hi, I am soldier" << endl;
}

Soldier::~Soldier() {}

void Soldier::job()
{
    if (this->step--) {
        // Stay around with Queen
        pos_t queenPos = getQueenPos();
        pos_t entropy(int(getRand() * 2), int(getRand() * 2));
        // Move around ```target``` pos
        pos_t target = queenPos + entropy;

        for (int i = 0; i < 5; i++) {
            go(this->pos, target);
            usleep(SLEEP_DURATION);
        }
    }
}

void Soldier::setProtectDistance(double pd)
{
    this->portectDistance = pd;
}

Worker::Worker(LocalMap *_myMap)
{
    this->myMap = _myMap;
    wandering();
    std::cout << "hi, I am worker" << endl;
}

Worker::~Worker() {}

// Make `Worker` forget it's orig pos
void Worker::wandering()
{
    // Random move a short path,
    int vertical = ceil(getRand() * STEP) * (getRand() > 0.5 ? 1 : -1);
    int horizent = ceil(getRand() * STEP) * (getRand() > 0.5 ? 1 : -1);

    pos_t wandering(vertical, horizent);
    while (pos != wandering && step--)
        go(pos, wandering);
}

void Worker::job()
{
    // There are some designed concepts
    // In the begining of the simulation, ants can only sensor food whatever far
    // (wherever) is the food It might not be a good implementation But after
    // some interval time, ancestors left the pheromone Descenders by means of
    // sensoring this pheromone to move , which follows real ant's behaviour.
    pos_t food = myMap->findClosest(pos, FOOD);
    pos_t ph = myMap->findClosest(pos, PHEROMONE);
    pos_t home = myMap->findClosest(pos, HOME);

    // Find the nearest object which can be sensored
    // `destination` when not taken food, care about FOOD
    // `wayHome` when taken food, its only responsibility is go home, care about
    // HOME
    pair<pos_t, char> destination;
    pair<pos_t, char> wayHome;

    if (ph < food)
        destination = make_pair(ph, PHEROMONE);
    else
        destination = make_pair(food, FOOD);

    if (ph < home)
        wayHome = make_pair(ph, PHEROMONE);
    else
        wayHome = make_pair(home, HOME);
    // Sensoring surrounding, check is there anything
    // Default to firstly trail pheromone
    // If (total ) step decrease down to 0, it will die
    if (step)
        die();
    // Not taken food
    else if (myObj.type == EMPTY) {
        // Arrival
        if (destination.first == pos) {
            // Arrive FOOD
            if (destination.second == FOOD) {
                myObj.type = FOOD;
                // Probability of take 1 or 2 food is 1/2, 1/2 respectively
                myObj.food = getRand() > 0.5 ? 1 : 2;
                myMap->merge(pos, MapObj(-myObj.food, myObj.type));
                // For recording is there any food at the place
                remaining = (myMap->get_at(pos).food > 0);
            }

        } else {
            // Leave the pheromone
            // FORMULA: how many step you moved ``%`` frequency
            // eg !(0 = (10) % `5`) putPheromone per `5` steps,
            // `5` is PHEROMONE_FREQUENCY
            if (!((MAXSTEP - step) % PHEROMONE_FREQUENCY))
                putPheromone();
            // move one step in a time
            step--;
            go(pos, destination.first);
        }
    }
    // If find food, take it and prepare to home
    // Find the route back home
    // Thought: back home do not need to put pheromone anyway
    else if (home == this->myQueen->at()) {
        // Home arrival
        if (destination.second == HOME && destination.first == pos) {
            myMap->merge(pos, MapObj(myObj.food, myObj.type));
            myObj.clean();
            remaining = false;
        }
        // PHEROMONE leave and wipe up
        // May be a bug, lead ant get lost
        //------------------------------------------------
        else if (destination.second == PHEROMONE)
            // Thought: if not remaining food ,do wipe pheromone up
            if (!remaining)
                myMap->merge(pos, MapObj());
        //------------------------------------------------

        step--;
        go(pos, wayHome.first);
    }
    // If nearest home is NOT its home
    // , head for near pheromone
    else {
        step--;
        go(pos, ph);
    }

    // Back home successfully, contribute to increase food
    // else, die on the road
    // For some convenience, if Worker ant dies, food disappears

    usleep(SLEEP_DURATION);
}

void Worker::putPheromone()
{
    // log_{1.1}^100 = 48.32
    // After 48 SLEEP_DURATION will disappear
    myMap->merge(pos, MapObj(100, PHEROMONE));
}
