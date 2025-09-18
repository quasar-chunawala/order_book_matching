#ifndef ORDER_H
#define ORDER_H

#include <stdexcept>
#include <memory>
#include <cstdint>
#include <format>
#include <string>
#include <utility>
#include "order_type.h"
#include "symbol.h"

namespace dev{
    using Price         = int64_t;
    using Quantity      = uint64_t;
    using OrderId       = uint64_t;      
    using Sequence      = uint32_t;
    using Side          = char;
    using UserId        = std::string;

    // VR
    // Data-oriented design
    // Mike Acton - Look up his videos. Good for high-performance.
    // Think about the data at design time. 
    //Order{ .order_id = 10, .price = 100 }
    struct Order{
        OrderType order_type;
        OrderId order_id;
        UserId user_id;
        Side side;
        Symbol symbol;
        Price price;
        Quantity initial_quantity;
        Quantity remaining_quantity;
    };
}
#endif
