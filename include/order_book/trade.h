#ifndef TRADE_H
#define TRADE_H

namespace dev {

struct TradeInfo;
struct Trade
{
    // Two trades that cross
    TradeInfo executing_order;
    TradeInfo reducing_order;
};
}
#endif