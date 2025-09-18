#ifndef TRADE_H
#define TRADE_H

#include "trade_info.h"

namespace dev{
    class Trade{
        public:
        Trade(const TradeInfo& bid_order, const TradeInfo& ask_order)
        : m_bid_order{ bid_order }
        , m_ask_order{ ask_order }
        {}

        [[nodiscard]] TradeInfo& get_bid_order() 
        {
            return m_bid_order;
        }

        [[nodiscard]] TradeInfo& get_ask_order()
        {
            return m_ask_order;
        }

        private:
        // Two trades that cross
        TradeInfo m_bid_order;
        TradeInfo m_ask_order;
    };
}
#endif