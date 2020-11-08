#include <iostream>
#include "../map.hpp"

int main()
{
    LocalMap l;
    l.put_at(_p(), MapObj(255));
    auto arr = l.show(1);
    for (auto i : arr) {
        for (auto j : i)
            std::cout << (j) << " ";
        std::cout << std::endl;
    }
    return 0;
}
