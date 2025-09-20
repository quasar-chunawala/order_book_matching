#include "market_data_manager.h"
#include "order.h"
#include "order_book.h"
#include "order_type.h"
#include "price_level.h"
#include "symbol.h"
#include "trade.h"
#include <format>
#include <gtest/gtest.h>
#include <string_view>

// Formatter for TradeInfo (already present in your file)
template<>
struct std::formatter<dev::TradeInfo> : std::formatter<std::string_view>
{
    auto format(const dev::TradeInfo& t, auto& ctx) const
    {
        std::string s = std::format("TradeInfo{{fill_type={}, user_id={}, order_id={}, "
                                    "symbol={}, price={}, quantity={}}}",
                                    t.fill_type == FillType::Full ? "Full" : "Partial",
                                    t.user_id,
                                    t.order_id,
                                    t.symbol.m_symbol_name,
                                    t.price,
                                    t.quantity);
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

using namespace dev;

TEST(order_book_tests, MarketOrder_Match_FullFill)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;

    // Buy order
    Order buy{ .order_type = OrderType::MARKET,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 100,
               .initial_quantity = 100,
               .remaining_quantity = 100 };

    // Sell order
    Order sell{ .order_type = OrderType::MARKET,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    auto trades1 = mm.add_market_order(buy);
    auto trades2 = mm.add_market_order(sell);

    // The match should occur after the second order
    ASSERT_TRUE(trades2.size() == 1);
    EXPECT_EQ(trades2[0].get_bid_order().quantity, 100);
    EXPECT_EQ(trades2[0].get_ask_order().quantity, 100);
    EXPECT_EQ(trades2[0].get_bid_order().fill_type, FillType::Full);
    EXPECT_EQ(trades2[0].get_ask_order().fill_type, FillType::Full);
}

TEST(order_book_tests, MarketOrder_Match_PartialFill)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;

    // Buy order (smaller quantity)
    Order buy{ .order_type = OrderType::MARKET,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 100,
               .initial_quantity = 50,
               .remaining_quantity = 50 };

    // Sell order (larger quantity)
    Order sell{ .order_type = OrderType::MARKET,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    auto trades1 = mm.add_market_order(buy);
    auto trades2 = mm.add_market_order(sell);

    ASSERT_TRUE(trades2.size() == 1);
    EXPECT_EQ(trades2[0].get_bid_order().quantity, 50);
    EXPECT_EQ(trades2[0].get_ask_order().quantity, 50);
    EXPECT_EQ(trades2[0].get_bid_order().fill_type, FillType::Full);
    EXPECT_EQ(trades2[0].get_ask_order().fill_type, FillType::Partial);
}

TEST(order_book_tests, MarketOrder_NoMatch)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;

    // Buy order at price 99
    Order buy{ .order_type = OrderType::MARKET,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 99,
               .initial_quantity = 100,
               .remaining_quantity = 100 };

    // Sell order at price 101
    Order sell{ .order_type = OrderType::MARKET,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 101,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    auto trades1 = mm.add_market_order(buy);
    auto trades2 = mm.add_market_order(sell);

    // No match should occur
    ASSERT_TRUE(trades2.empty());
}

TEST(order_book_tests, MarketOrder_MultipleMatches)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;

    // Add two buy orders
    Order buy1{ .order_type = OrderType::MARKET,
                .order_id = 1,
                .user_id = "buyer1",
                .side = 'B',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 50,
                .remaining_quantity = 50 };

    Order buy2{ .order_type = OrderType::MARKET,
                .order_id = 2,
                .user_id = "buyer2",
                .side = 'B',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 50,
                .remaining_quantity = 50 };

    mm.add_market_order(buy1);
    mm.add_market_order(buy2);

    // Add a sell order that matches both buys
    Order sell{ .order_type = OrderType::MARKET,
                .order_id = 3,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    auto trades = mm.add_market_order(sell);

    ASSERT_TRUE(trades.size() == 2);
    EXPECT_EQ(trades[0].get_bid_order().user_id, "buyer1");
    EXPECT_EQ(trades[1].get_bid_order().user_id, "buyer2");
    EXPECT_EQ(trades[0].get_ask_order().user_id, "seller");
    EXPECT_EQ(trades[1].get_ask_order().user_id, "seller");
    EXPECT_EQ(trades[0].get_bid_order().quantity, 50);
    EXPECT_EQ(trades[1].get_bid_order().quantity, 50);
    EXPECT_EQ(trades[0].get_ask_order().quantity, 50);
    EXPECT_EQ(trades[1].get_ask_order().quantity, 50);
}