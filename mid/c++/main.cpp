#include "ant.hpp"
#include "map.hpp"

int main()
{
    LocalMap localMap;
    Ant myant[5] = {
        Ant(1), Ant(2), Ant(3), Ant(4), Ant(5),
    };
    for (size_t i = 0; i < 5; i++) {
        myant[i].job();
    }
    return 0;
}