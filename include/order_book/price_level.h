#ifndef PRICE_LEVEL_H
#define PRICE_LEVEL_H

#include <algorithm>
#include <cstdint>
#include <deque>
#include <format>
#include <numeric>
#include <optional>
#include <vector>

namespace dev {

using FreeList = std::deque<SeqNum>;
class MarketDataManager;

enum class LevelType
{
    BID,
    ASK
};

/**
 * @brief A PriceLevel is a queue of open orders at a given price.
 */
class PriceLevel
{
  public:
    // Constructors
    explicit PriceLevel(LevelType type, Price price, OrderBook& order_book);

    // Getters
    LevelType get_level_type() const;
    Price get_price() const;
    SeqNum get_first_seq_num();
    SeqNum get_last_seq_num();
    Order& front();
    Order& back();

    // Validation
    bool can_fill(Order order);
    bool is_empty();

    // Modifiers
    void on_empty_helper(Order order);
    void push_front(Order order);
    void pop_front();
    void push_back(Order order);
    void pop_back();
    void fill_order(Order order);

  private:
    // Level type
    LevelType m_level_type;

    // Level price
    Price m_price;

    // Each PriceLevel is a pair of start and end indices into the
    // the order_pool.
    SeqNum m_first_seq_num;
    SeqNum m_last_seq_num;

    // Reference to the order_pool and free_list
    OrderBook& m_order_book;
};

using PriceLevels = std::vector<PriceLevel>;
} // namespace dev

#endif