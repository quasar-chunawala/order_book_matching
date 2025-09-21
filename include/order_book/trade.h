#ifndef TRADE_H
#define TRADE_H

#include "trade_info.h"

namespace dev {
struct Trade
{
    // Two trades that cross
    TradeInfo executing_order;
    TradeInfo reducing_order;
};
}
#endif