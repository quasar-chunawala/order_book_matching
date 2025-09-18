#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <list>
#include <chrono>
#include <unordered_map>
#include "order_type.h"
#include "market_data_manager.h"
#include "order_book.h"
#include "order.h"
#include "price_level.h"
#include "symbol.h"
#include "trade.h"
#include <gtest/gtest.h>

TEST(order_book_tests, AddOrderTest)
{
    using namespace dev;
    Symbol MSFT{1, "MSFT"};

    std::vector<Order> orders{
        Order{
            .order_type = OrderType::MARKET,
            .order_id = 1,
            .user_id = "user001",
            .side = 'B',
            .symbol = MSFT,
            .price = 100,
            .initial_quantity = 50,
            .remaining_quantity = 50
        },
        Order{
            .order_type = OrderType::MARKET,
            .order_id = 2,
            .user_id = "user002",
            .side = 'S',
            .symbol = MSFT,
            .price = 100,
            .initial_quantity = 50,
            .remaining_quantity = 50
        }
    };

    MarketDataManager mm;
    Trades trades{};
    for(auto& o : orders)
    {
        mm.add_market_order(o);
    }

    ASSERT_EQ(true, true);
}