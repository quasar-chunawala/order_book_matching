#ifndef ORDER_H
#define ORDER_H

#include "order_id.h"
#include "order_type.h"
#include "usings.h"
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace dev {

// Data-oriented design
struct Order
{
    OrderType order_type;
    OrderId order_id;
    UserId user_id;
    Side side;
    Price price;
    Quantity initial_quantity;
    Quantity remaining_quantity;
};
}
#endif
