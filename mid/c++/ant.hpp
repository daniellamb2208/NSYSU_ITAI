#ifndef __ANT_HPP__
#define __ANT_HPP__

#include <utility>
#include "map.hpp"
using namespace std;


class Ant
{
private:
    pos_t pos;
    int step;
    int obj_value;

public:
    Ant();
    Ant(pos_t _pos, int _step, int opcode);
    ~Ant();
    pos_t getPos();
    void setStep(int step);
    int getStep();
    // declare a prototype of job, will impelement it by the other
    bool (*job)();
    void die();
    void setObjValue();
    int getObjValue();
    void sync();
};

#endif