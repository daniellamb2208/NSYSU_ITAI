#include "ant.hpp"
#include <iostream>
using namespace std;

bool behavior1()
{
    cout << "1" << endl;
    return true;
}
bool behavior2()
{
    cout << "2" << endl;
    return true;
}
bool behavior3()
{
    cout << "3" << endl;
    return true;
}
bool behavior4()
{
    cout << "4" << endl;
    return true;
}
bool behavior5()
{
    cout << "5" << endl;
    return true;
}

// opcode is in [1, 5]
// Return a function pointer of a job
auto jobSelector(int opcode)
{
    bool (*func[5])() = {behavior1, behavior2, behavior3, behavior4, behavior5};
    return func[opcode];
}

Ant::Ant(pos_t _pos, int _step, int opcode)
{
    this->job = jobSelector(opcode);
    this->step = _step;
    this->pos = _pos;
}

Ant::~Ant() {}