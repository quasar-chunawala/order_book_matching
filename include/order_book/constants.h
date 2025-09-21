#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cmath>
#include <cstdint>
#include <limits>

using Price = uint64_t;
struct Constants
{
    static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
};
#endif