#ifndef FORMATTER_H
#define FORMATTER_H

#include <format>
#include <string_view>

template<>
struct std::formatter<dev::TradeInfo> : std::formatter<std::string_view>
{
    auto format(const dev::TradeInfo& t, auto& ctx) const
    {
        std::string s = std::format("TradeInfo{{fill_type={}, user_id={}, seq_num={}, "
                                    "symbol={}, price={}, filled_quantity={}}}",
                                    t.fill_type == FillType::Full ? "Full" : "Partial",
                                    t.user_id,
                                    t.order_id.seq_num,
                                    t.order_id.symbol_name,
                                    t.price,
                                    t.quantity);
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

template<>
struct std::formatter<dev::OrderId> : std::formatter<std::string_view>
{
    auto format(const dev::OrderId& order_id, auto& ctx) const
    {
        std::string s = std::format(
          "OrderId{{SeqNum={}, symbol_name={}}}", order_id.seq_num, order_id.symbol_name);
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

#endif