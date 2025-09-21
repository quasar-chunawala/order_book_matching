#ifndef PRICE_LEVEL_H
#define PRICE_LEVEL_H

#include "order.h"
#include <algorithm>
#include <cstdint>
#include <deque>
#include <format>
#include <numeric>
#include <optional>
#include <vector>

namespace dev {

using OrderQueue = std::deque<Order>;

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
  private:
    // Level type
    LevelType m_type;
    // Level price
    Price m_price;

    // Total volume at this price
    std::size_t m_total_volume;
    /* std::deque is just a dynamic array of pointers to chunks
    of contigous memory. It is automatically expanded and contracted
    as needed. It supports random access as well as O(1) constant-time
    insertion and removal of elements at either end. */
    std::deque<Order> m_price_level_orders;

    auto get_order_iter(OrderId order_id)
    {
        auto it = std::lower_bound(
          m_price_level_orders.begin(),
          m_price_level_orders.end(),
          order_id,
          [](const Order& order, OrderId value) { return order.order_id < value; });

        return it;
    }

  public:
    PriceLevel() = default;
    PriceLevel(LevelType type,
               Price price,
               const std::deque<Order>& orders = std::deque<Order>{})
      : m_type{ type }
      , m_price{ price }
      , m_price_level_orders{ orders }
    {
        m_total_volume = std::accumulate(
          orders.begin(), orders.end(), 0, [](std::size_t accum, Order order) {
              accum += order.remaining_quantity;
              return accum;
          });
    }

    void swap(PriceLevel& other) noexcept
    {
        std::swap(m_type, other.m_type);
        std::swap(m_price, other.m_price);
        std::swap(m_total_volume, other.m_total_volume);
        std::swap(m_price_level_orders, other.m_price_level_orders);
    }

    PriceLevel(PriceLevel&& other) noexcept
      : m_type{ std::exchange(other.m_type, LevelType::BID) }
      , m_price{ std::exchange(other.m_price, 0) }
      , m_total_volume{ std::exchange(other.m_total_volume, 0) }
      , m_price_level_orders{ std::exchange(other.m_price_level_orders, {}) }
    {
    }

    PriceLevel& operator=(PriceLevel&& other) noexcept
    {
        PriceLevel(std::move(other)).swap(*this);
        return *this;
    }
    LevelType get_level_type() const { return m_type; }

    LevelType get_level_type() { return m_type; }

    Price get_price() const { return m_price; }

    std::size_t get_count() const { return m_price_level_orders.size(); }

    std::size_t get_total_volume() const { return m_total_volume; }

    Order& get_order(OrderId order_id)
    {
        auto it = std::lower_bound(
          m_price_level_orders.begin(),
          m_price_level_orders.end(),
          order_id,
          [=](Order& order, OrderId value) { return order.order_id < value; });

        if (it == m_price_level_orders.end()) {
            throw std::logic_error("Error fetching the requested Order Id!");
        }

        return *it;
    }

    void add_order(Order order)
    {
        m_price_level_orders.push_back(order);
        m_total_volume += order.initial_quantity;
    }

    void modify_order(OrderId order_id, Quantity quantity)
    {
        auto it = get_order_iter(order_id);
        if (it == m_price_level_orders.end()) {
            throw std::logic_error("Error fetching the requested Order Id!");
        }

        m_total_volume = m_total_volume - it->remaining_quantity + quantity;
        it->initial_quantity = quantity;
        it->remaining_quantity = quantity;
    }

    void cancel_order(OrderId order_id)
    {
        auto it = get_order_iter(order_id);
        if (it == m_price_level_orders.end()) {
            throw std::logic_error("Error fetching the requested Order Id!");
        }

        Quantity cancel_quantity = it->remaining_quantity;
        m_price_level_orders.erase(it);
        m_total_volume -= cancel_quantity;
    }

    void pop_front()
    {
        Order& order_at_front = m_price_level_orders.front();
        m_price_level_orders.pop_front();
        m_total_volume -= order_at_front.remaining_quantity;
    }

    void fill_order(Order& order)
    {
        Order& reducing_order = order;
        Order& executing_order = m_price_level_orders.front();
        Quantity fill_quantity =
          std::min(reducing_order.remaining_quantity, executing_order.remaining_quantity);

        // Fill against the executing_order
        if (executing_order.remaining_quantity > reducing_order.remaining_quantity)
            std::swap(executing_order, reducing_order);

        reducing_order.remaining_quantity -= fill_quantity;
        executing_order.remaining_quantity -= fill_quantity;
        m_total_volume -= fill_quantity;

        if (executing_order.order_id == m_price_level_orders.front().order_id) {
            m_price_level_orders.pop_front();
        }
    }

    Order& front() { return m_price_level_orders.front(); }

    bool is_empty() { return m_price_level_orders.empty(); }
};

using PriceLevels = std::vector<PriceLevel>;
} // namespace dev

#endif