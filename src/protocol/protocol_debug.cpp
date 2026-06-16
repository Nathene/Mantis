#include "mantis/protocol.hpp"

#include <iomanip>
#include <sstream>
#include <cmath>

namespace Protocol {

// Does allocate on the heap however this is only used for debugging purposes
std::string format_price(int64_t ticks) {
    bool negative = ticks < 0;
    std::int64_t abs_ticks = std::abs(ticks);

    std::int64_t whole = abs_ticks / PRICE_SCALE;
    std::int64_t fractional = abs_ticks % PRICE_SCALE;

    std::ostringstream oss;
    if (negative) oss << "-";

    oss << whole << "." << std::setfill('0') << std::setw(TICK_PRECISION) << fractional;

    std::string s = oss.str();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') s += '0';

    return s;
}

}
