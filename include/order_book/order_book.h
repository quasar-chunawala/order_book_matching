#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "order_node.h"
#include "price_level.h"
#include "trade.h"
#include "trade_info.h"
#include "usings.h"
#include <algorithm>
#include <chrono>
#include <deque>
#include <format>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dev {

class MarketDataManager; // forward declaration (avoid including manager header)

using Trades = std::vector<Trade>;
using PriceLevels = std::vector<PriceLevel>;

enum class LevelType;

/**
 * @brief An @a OrderBook is a data-structure that is an ordered collection bids and asks
 * arranged by price-time priority.
 */
class OrderBook
{
    friend class PriceLevel;

  public:
    // Constructors and assignment operators
    explicit OrderBook();
    // void swap(OrderBook& other);
    // OrderBook(OrderBook&& other);
    // OrderBook& operator=(OrderBook&&);

    // Getters
    Order& get_order(OrderId order_id);
    PriceLevels& get_bids();
    PriceLevels& get_asks();
    PriceLevels& get_price_levels(LevelType level_type);
    PriceLevel& get_best_bid();
    PriceLevel& get_best_ask();
    const PriceLevel& get_best_bid() const;
    const PriceLevel& get_best_ask() const;

    auto get_price_level_iter(LevelType level_type, Price price);
    PriceLevel& get_bid_price_level(Price price);
    PriceLevel& get_ask_price_level(Price price);
    PriceLevel& get_price_level(LevelType level_type, Price price);
    std::deque<SeqNum>& get_free_list();

    bool is_match_possible(Side side, Price price);
    bool order_exists(OrderId order_id);

    // Modifiers
    void add_order(OrderType order_type,
                   UserId user_id,
                   Side side,
                   std::string_view symbol_name,
                   Price price,
                   Quantity quantity);
    void modify_order(OrderId order_id, Price new_price, Quantity new_quantity);
    void cancel_order(OrderId order_id);
    void match();
    void add_price_level(LevelType level_type, Price price);
    void delete_price_level(LevelType level_type, Price price);

    SeqNum get_next_seq_num();
    OrderId generate_order_id(std::string_view symbol_name);

  private:
    // All incoming orders are stored sequentially in an order_pool.
    // The order_pool is a pre-allocated buffer and consists of OrderNodes.
    std::vector<OrderNode> m_order_pool;

    // List of free indices
    std::deque<SeqNum> m_free_list;

    // Buy orders sorted by price (highest first)
    std::vector<PriceLevel> m_bids;
    // Sell orders sorted by price (lowest first)
    std::vector<PriceLevel> m_asks;

    PriceLevels::iterator find_insert_location(PriceLevels& price_levels,
                                               LevelType level_type,
                                               Price price);
};

using OrderBooks = std::unordered_map<size_t, OrderBook>;
} // namespace dev

#endif
