#include "formatter.h"
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

using namespace dev;

TEST(order_book_tests, LimitOrder_Match_FullFill)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;
    mm.add_order_book(MSFT);

    Order buy{ .order_type = OrderType::LIMIT,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 100,
               .initial_quantity = 100,
               .remaining_quantity = 100 };

    Order sell{ .order_type = OrderType::LIMIT,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    mm.add_order(buy);
    auto trades = mm.add_order(sell);

    ASSERT_EQ(trades.size(), 1);

    std::string expected_bid =
      std::format("{}", TradeInfo{ FillType::Full, "buyer", 1, MSFT, 100, 100 });
    std::string expected_ask =
      std::format("{}", TradeInfo{ FillType::Full, "seller", 2, MSFT, 100, 100 });

    EXPECT_EQ(std::format("{}", trades[0].executing_order), expected_bid);
    EXPECT_EQ(std::format("{}", trades[0].reducing_order), expected_ask);
}

TEST(order_book_tests, LimitOrder_Match_PartialFill)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;
    mm.add_order_book(MSFT);

    Order buy{ .order_type = OrderType::LIMIT,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 100,
               .initial_quantity = 50,
               .remaining_quantity = 50 };

    Order sell{ .order_type = OrderType::LIMIT,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    mm.add_order(buy);
    auto trades = mm.add_order(sell);

    ASSERT_EQ(trades.size(), 1);

    std::string expected_bid =
      std::format("{}", TradeInfo{ FillType::Full, "buyer", 1, MSFT, 100, 50 });
    std::string expected_ask =
      std::format("{}", TradeInfo{ FillType::Partial, "seller", 2, MSFT, 100, 50 });

    EXPECT_EQ(std::format("{}", trades[0].executing_order), expected_bid);
    EXPECT_EQ(std::format("{}", trades[0].reducing_order), expected_ask);
}

TEST(order_book_tests, LimitOrder_NoMatch)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;
    mm.add_order_book(MSFT);

    Order buy{ .order_type = OrderType::LIMIT,
               .order_id = 1,
               .user_id = "buyer",
               .side = 'B',
               .symbol = MSFT,
               .price = 99,
               .initial_quantity = 100,
               .remaining_quantity = 100 };

    Order sell{ .order_type = OrderType::LIMIT,
                .order_id = 2,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 101,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    mm.add_order(buy);
    auto trades = mm.add_order(sell);

    ASSERT_TRUE(trades.empty());
}

TEST(order_book_tests, LimitOrder_MultipleMatches)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;
    mm.add_order_book(MSFT);

    Order buy1{ .order_type = OrderType::LIMIT,
                .order_id = 1,
                .user_id = "buyer1",
                .side = 'B',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 50,
                .remaining_quantity = 50 };

    Order buy2{ .order_type = OrderType::LIMIT,
                .order_id = 2,
                .user_id = "buyer2",
                .side = 'B',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 50,
                .remaining_quantity = 50 };

    mm.add_order(buy1);
    mm.add_order(buy2);

    Order sell{ .order_type = OrderType::LIMIT,
                .order_id = 3,
                .user_id = "seller",
                .side = 'S',
                .symbol = MSFT,
                .price = 100,
                .initial_quantity = 100,
                .remaining_quantity = 100 };

    auto trades = mm.add_order(sell);

    ASSERT_EQ(trades.size(), 2);

    std::string expected_bid1 =
      std::format("{}", TradeInfo{ FillType::Full, "buyer1", 1, MSFT, 100, 50 });
    std::string expected_bid2 =
      std::format("{}", TradeInfo{ FillType::Full, "buyer2", 2, MSFT, 100, 50 });
    std::string expected_ask1 =
      std::format("{}", TradeInfo{ FillType::Partial, "seller", 3, MSFT, 100, 50 });
    std::string expected_ask2 =
      std::format("{}", TradeInfo{ FillType::Partial, "seller", 3, MSFT, 100, 50 });

    EXPECT_EQ(std::format("{}", trades[0].executing_order), expected_bid1);
    EXPECT_EQ(std::format("{}", trades[1].executing_order), expected_bid2);
    EXPECT_EQ(std::format("{}", trades[0].reducing_order), expected_ask1);
    EXPECT_EQ(std::format("{}", trades[1].reducing_order), expected_ask2);
}

TEST(order_book_tests, MarketOrder_Fill)
{
    Symbol MSFT{ 1, "MSFT" };
    MarketDataManager mm;
    mm.add_order_book(MSFT);

    std::vector<Order> orders{
        Order{ OrderType::LIMIT, 1, "buyer1", 'B', MSFT, 95, 50, 50 },
        Order{ OrderType::LIMIT, 2, "buyer2", 'B', MSFT, 100, 50, 50 },
        Order{ OrderType::LIMIT, 3, "buyer3", 'B', MSFT, 105, 50, 50 },
        Order{
          OrderType::MARKET, 4, "seller", 'S', MSFT, Constants::InvalidPrice, 50, 125 },
    };

    Trades trades{}, head{}, tail{};
    for (auto& o : orders) {
        tail = mm.add_order(o);
        head.insert(head.end(), tail.begin(), tail.end());
    }
    trades = head;
    EXPECT_EQ(std::format("{}", trades[0].executing_order),
              "TradeInfo{fill_type=Full, user_id=buyer3, order_id=3, symbol=MSFT, "
              "price=105, filled_quantity=50}");
    EXPECT_EQ(std::format("{}", trades[0].reducing_order),
              "TradeInfo{fill_type=Partial, user_id=seller, order_id=4, symbol=MSFT, "
              "price=95, filled_quantity=50}");
    EXPECT_EQ(std::format("{}", trades[1].executing_order),
              "TradeInfo{fill_type=Full, user_id=buyer2, order_id=2, symbol=MSFT, "
              "price=100, filled_quantity=50}");
    EXPECT_EQ(std::format("{}", trades[1].reducing_order),
              "TradeInfo{fill_type=Partial, user_id=seller, order_id=4, symbol=MSFT, "
              "price=95, filled_quantity=50}");
    EXPECT_EQ(std::format("{}", trades[2].executing_order),
              "TradeInfo{fill_type=Full, user_id=seller, order_id=4, symbol=MSFT, "
              "price=95, filled_quantity=25}");
    EXPECT_EQ(std::format("{}", trades[2].reducing_order),
              "TradeInfo{fill_type=Partial, user_id=buyer1, order_id=1, symbol=MSFT, "
              "price=95, filled_quantity=25}");
}
