#ifndef FORMATTER_H
#define FORMATTER_H

#include "trade_info.h"
#include <format>
#include <string_view>

template<>
struct std::formatter<dev::TradeInfo> : std::formatter<std::string_view>
{
    auto format(const dev::TradeInfo& t, auto& ctx) const
    {
        std::string s = std::format("TradeInfo{{fill_type={}, user_id={}, order_id={}, "
                                    "symbol={}, price={}, filled_quantity={}}}",
                                    t.fill_type == FillType::Full ? "Full" : "Partial",
                                    t.user_id,
                                    t.order_id,
                                    t.symbol.m_symbol_name,
                                    t.price,
                                    t.quantity);
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

#endif