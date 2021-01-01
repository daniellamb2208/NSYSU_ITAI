#include <ostream>
namespace Color
{
enum Code {
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_DEFAULT = 49
};
class Modifier
{
    Code code;

public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream &operator<<(std::ostream &os, const Modifier &mod)
    {
        return os << "\033[" << mod.code << "m";
    }
};

Color::Modifier red(Color::FG_RED);
Color::Modifier green(Color::FG_GREEN);
Color::Modifier blue(Color::FG_BLUE);
Color::Modifier yellow(Color::FG_YELLOW);
Color::Modifier default_c(Color::FG_DEFAULT);

}  // namespace Color



// Four colors
// enum { EMPTY, FOOD, PHEROMONE, HOME };