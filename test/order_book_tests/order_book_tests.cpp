#include "formatter.h"
#include "market_data_manager.h"
#include "order.h"
#include "order_book.h"
#include "order_type.h"
#include "price_level.h"
#include "trade.h"
#include <format>
#include <gtest/gtest.h>
#include <string_view>

using namespace dev;

TEST(order_book_tests, LimitOrder_Match_FullFill)
{
    MarketDataManager mm;
    mm.add_order(OrderType::LIMIT, "buyer01", 'B', "MSFT", 100, 50);
}