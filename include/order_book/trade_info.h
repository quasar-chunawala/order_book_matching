#ifndef TRADEINFO_H
#define TRADEINFO_H
#include "order.h"

enum class FillType
{
    Partial,
    Full,
};

namespace dev {
struct TradeInfo
{
    FillType fill_type;
    UserId user_id;
    OrderId order_id;
    Symbol symbol;
    Price price;
    Quantity quantity;
};
}
#endif