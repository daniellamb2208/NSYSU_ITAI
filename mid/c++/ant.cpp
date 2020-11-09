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

template <typename Base, typename T>
inline bool instanceof (const T &)
{
    return std::is_base_of<Base, T>::value;
}

Ant::~Ant() {}

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
    if (this->myObj.value > 0)
        myMap->merge(this->pos, this->myObj);
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
    obj.value = -this->appetite;
    this->myMap->merge(this->pos, obj);
}

NonBreedAnt::NonBreedAnt(LocalMap *_myMap)
{
    myMap = _myMap;
}

NonBreedAnt::~NonBreedAnt() {}

void NonBreedAnt::take(MapObj obj)
{
    // Take constant quantity
    this->myObj = obj;
    this->myObj.food = 2;
    obj.food = -2;
    this->myMap->merge(this->pos, obj);  // put it back
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
    this->setAppetite(2);
}

Queen::~Queen()
{
    auto iter = this->queenPtrPool->begin();
    while ((iter++)->get() != this)
        ;
    this->queenPtrPool->erase(--iter);
}

int Queen::pregnant()
{
    // Number of children
    int num = int(getRand() * 10 + CHILDREN_BASE);
    auto setAttr = [this](shared_ptr<Ant> prototype) {
        prototype->setMap(myMap);
        prototype->at() = pos;
        prototype->setStep(MAXSTEP);
    };
    for (int i = 0; i < num && (myMap->get_at(pos).value > 0); i++) {
        this->eat();

        // 2% Virgin, 18% Male, 20% Soldier, 60% Worker
        auto selectIndex = int(floor(getRand() * 100));
        if (selectIndex < 0) {  // Virgin
            shared_ptr<Virgin> prototype = make_shared<Virgin>();
            setAttr(prototype);

            prototype->setQPP(this->queenPtrPool);
            prototype->setAppetite(2);
            auto partial = this->myMap->get_at(this->pos);
            partial.value /= 20;  // pick up some orig value
            prototype->myObj = partial;
            prototype->pos = pos_t(getRand() * HEIGHT, getRand() * WIDTH);
            prototype->slave = vector<shared_ptr<Ant>>();
            prototype->slave.clear();
            prototype->queenPtrPool->push_back(prototype);
            continue;
        } else if (selectIndex < 20) {  // Male
            shared_ptr<Male> prototype = make_shared<Male>();
            setAttr(prototype);
            prototype->setAppetite(0.7);
            prototype->setMyQueen(this);
            this->slave.push_back(prototype);
        } else if (selectIndex < 40) {  // Soldier
            shared_ptr<Soldier> prototype = make_shared<Soldier>();
            setAttr(prototype);
            prototype->setMyQueen(this);
            prototype->setProtectDistance(10);
            this->slave.push_back(prototype);

        } else {  // Worker
            shared_ptr<Worker> prototype = make_shared<Worker>();
            setAttr(prototype);
            prototype->setMyQueen(this);
            this->slave.push_back(prototype);
        }
        usleep(SLEEP_DURATION);
    }
    return num;
}

vector<shared_ptr<Ant>> &Queen::getSlave()
{
    return this->slave;
}

// Thoughts: food centrally controlled by Queen ant
void Queen::job()
{
    if (this->myMap->get_at(this->pos).food > 0) {
        this->eat();
        if (findMale()) {
            pregnant();
        } else {  // If not found Male, go sleep and make a `Male`
            usleep(SLEEP_DURATION);
            shared_ptr<Male> prototype = make_shared<Male>();
            prototype->setMap(myMap);
            prototype->at() = pos;
            prototype->setStep(MAXSTEP);
            prototype->setMyQueen(this);
            prototype->setAppetite(0.7);
            this->slave.push_back(prototype);
        }
    } else {
        cout << "Die" << endl;
        this->die();
        // FIXME : UGLY
        // this->~Queen();
    }
}

inline const bool Queen::findMale()
{
    // Sensor if any Male ant on itself(position overlap)
    // if sensored, mate with it, and Male ant will die

    for (auto iter = this->slave.begin(); iter != this->slave.end(); iter++) {
        if (dynamic_cast<Male *>(iter->get()) != nullptr &&
            iter->get()->at() == this->pos) {
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
        auto entropy = getRand() * 3;
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
}

Virgin::~Virgin() {}

Soldier::Soldier(LocalMap *_myMap)
{
    myMap = _myMap;
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
    if (this->step--) {
        pos_t target;
        if (this->myObj.type == EMPTY) {
            auto closestFood = this->myMap->findClosest(this->pos, FOOD);
            auto closestPhermone =
                this->myMap->findClosest(this->pos, PHEROMONE);
            if (closestPhermone == pos_t(-1, -1))  // Not found
                target = closestFood;
            else {
                auto phermone = this->myMap->get_at(closestPhermone);
                pos_t weightedPhermone(
                    closestPhermone.x * log10(phermone.value) / 10,
                    closestPhermone.y * log10(phermone.value) / 10);
                target = closestFood + weightedPhermone;
            }
            // Is arrived food, pick up to myObj
            if (this->pos == closestFood && this->myObj.type != HOME) {
                this->take(this->myMap->get_at(pos));
            }
        }
        if (this->myObj.type == FOOD) {  // Go home
            target = this->myQueen->at();
            // Every time period put PHEROMONE
//            if (!(this->step % PHEROMONE_FREQUENCY))
                putPheromone();
            if (this->pos == target) {
                this->myMap->merge(this->pos, this->myObj);
                this->myObj.clean();
            }
        }
        go(this->pos, target);
    }
    usleep(SLEEP_DURATION);
}

void Worker::putPheromone()
{
    // log_{1.1}^10 = 24.1588
    // After 24 SLEEP_DURATION will disappear
    myMap->merge(pos, MapObj(10, PHEROMONE));
}
