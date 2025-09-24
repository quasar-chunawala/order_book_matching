#ifndef ORDERNODE_H
#define ORDERNODE_H

#include "order.h"
#include <optional>
#include <vector>

namespace dev {

struct OrderNode
{
    std::optional<Order> order;
    size_t prev, next;
};

using OrderPool = std::vector<OrderNode>;
}

#endif