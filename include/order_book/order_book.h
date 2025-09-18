#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "order.h"
#include "order_type.h"
#include "price_level.h"
#include "symbol.h"
#include "trade.h"
#include <algorithm>
#include <chrono>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace dev {

using OrderQueue = std::deque<Order>;
using Trades = std::vector<Trade>;
using OrderPtr = Order*;
using PriceLevels = std::vector<PriceLevel>;

struct OrderEntry
{
    OrderId order_id;
    PriceLevel* price_level;
};

class OrderBook
{

    auto find_insert_location(const std::vector<PriceLevel>& price_levels, Price price)
    {
        // Empty queue
        if (price_levels.size() == 0)
            return price_levels.end();

        auto low = price_levels.begin();
        auto high = price_levels.end();
        auto mid = price_levels.begin() + std::distance(low, high) / 2;

        while (low < high) {
            if (price < mid->get_price())
                high = mid;

            if (price > mid->get_price())
                low = mid;

            if (price == mid->get_price()) {
                throw std::logic_error(
                  "Price level is already present in the order book");
            }
        }

        return high;
    }

  public:
    OrderId generate_order_id()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
          .count();
    }

    OrderBook(Symbol symbol)
      : m_symbol{ symbol }
    {
    }

    void swap(OrderBook& other)
    {
        std::swap(m_symbol, other.m_symbol);
        std::swap(m_bids, other.m_bids);
        std::swap(m_asks, other.m_asks);
        std::swap(m_order_entries, other.m_order_entries);
    }

    OrderBook(OrderBook&& other)
      : m_symbol{ std::exchange(other.m_symbol, "") }
      , m_bids{ std::exchange(other.m_bids, {}) }
      , m_asks{ std::exchange(other.m_asks, {}) }
      , m_order_entries{ std::exchange(other.m_order_entries, {}) }
    {
    }

    OrderBook& operator=(OrderBook&& other)
    {
        OrderBook(std::move(other)).swap(*this);
        return *this;
    }

    void add_price_level_helper(LevelType level_type,
                                Price price,
                                std::vector<PriceLevel>& price_levels,
                                const OrderQueue& order_queue)
    {
        if (price_levels.size() == 0) {
            m_bids.emplace_back(level_type, price, order_queue);
            return;
        }

        auto loc = find_insert_location(price_levels, price);
        m_bids.insert(loc, PriceLevel(level_type, price, order_queue));
    }
    /**
     * @brief Adds a price level
     */
    void add_price_level(LevelType type,
                         Price price,
                         const OrderQueue& order_queue = std::deque<Order>{})
    {
        if (type == LevelType::BID) {
            add_price_level_helper(LevelType::BID, price, m_bids, order_queue);
        } else {
            add_price_level_helper(LevelType::ASK, price, m_asks, order_queue);
        }
    }

    void delete_price_level_helper(Price price, std::vector<PriceLevel>& price_levels)
    {
        auto pos = std::lower_bound(price_levels.begin(),
                                    price_levels.end(),
                                    price,
                                    [](PriceLevel& price_level, Price value) {
                                        return price_level.get_price() < value;
                                    });

        if (pos != price_levels.end()) {
            m_bids.erase(pos);
        }
    }

    /**
     * @brief Deletes a price level
     */
    void delete_price_level(LevelType type, Price price)
    {
        if (type == LevelType::BID) {
            delete_price_level_helper(price, m_bids);
        } else {
            delete_price_level_helper(price, m_asks);
        }
    }

    PriceLevel* get_bid_price_level(Price price)
    {
        return get_price_level(LevelType::BID, price);
    }

    PriceLevel* get_ask_price_level(Price price)
    {
        return get_price_level(LevelType::ASK, price);
    }

    PriceLevel* get_price_level(LevelType level_type, Price price)
    {
        PriceLevels& price_levels = (level_type == LevelType::BID) ? m_bids : m_asks;
        // Find the price level matching the order price
        auto pos = std::lower_bound(price_levels.begin(),
                                    price_levels.end(),
                                    price,
                                    [](PriceLevel& price_level, Price value) {
                                        return price_level.get_price() < value;
                                    });

        if (pos == price_levels.end()) {
            return nullptr;
        }
        return std::addressof(*pos);
    }

    bool is_match_possible(Side side, Price price)
    {
        if (side == 'B') {
            if (m_asks.empty())
                return false;

            if (price < m_asks.back().get_price())
                return false;

            if (m_asks.back().get_count() == 0)
                return false;
        } else {
            if (m_bids.empty())
                return false;

            if (price > m_bids.back().get_price())
                return false;

            if (m_bids.back().get_count() == 0)
                return false;
        }
        return true;
    }

    // Order management API
    /**
     * @brief Add an order to the order book
     */
    Trades add_order(const Order& order)
    {
        Side side = order.side;
        Price price = order.price;
        LevelType level_type = side == 'B' ? LevelType::BID : LevelType::ASK;
        PriceLevel* price_level = get_price_level(level_type, price);

        if (!price_level) {
            add_price_level(level_type, price);
            price_level = get_price_level(level_type, price);
        }

        if (order.order_type == OrderType::FILL_OR_KILL &&
            !is_match_possible(order.side, order.price))
            return {};

        price_level->add_order(order);
        m_order_entries.push_back(
          OrderEntry{ .order_id = order.order_id, .price_level = price_level });
        return match();
    }

    const PriceLevel* get_best_bid() const { return std::addressof(m_bids.back()); }

    PriceLevel* get_best_bid() { return std::addressof(m_bids.back()); }

    const PriceLevel* get_best_ask() const { return std::addressof(m_asks.back()); }

    PriceLevel* get_best_ask() { return std::addressof(m_asks.back()); }

    auto get_order_entry(OrderId order_id)
    {
        auto order_entry_iter =
          std::lower_bound(m_order_entries.begin(),
                           m_order_entries.end(),
                           order_id,
                           [](OrderEntry& order_entry, OrderId value) {
                               return order_entry.order_id < value;
                           });
        return order_entry_iter;
    }

    /**
     * @brief The global match method attempts match orders
     * in priority of (price, arrival time).
     */
    Trades match()
    {
        Trades trades{};

        while (true) {
            if (m_bids.empty() || m_asks.empty())
                break;

            PriceLevel& best_bid_price_level = m_bids.back();
            PriceLevel& best_ask_price_level = m_asks.back();

            if (best_bid_price_level.get_price() < best_ask_price_level.get_price())
                break;

            while (best_bid_price_level.get_count() && best_ask_price_level.get_count()) {
                auto& bid_ = best_bid_price_level.front();
                auto& ask_ = best_ask_price_level.front();
                Quantity fill_quantity =
                  std::min(bid_.remaining_quantity, ask_.remaining_quantity);

                auto& executing_order = bid_;
                auto& reducing_order = ask_;
                if (executing_order.remaining_quantity >
                    reducing_order.remaining_quantity) {
                    std::swap(executing_order, reducing_order);
                }

                if (executing_order.order_id == bid_.order_id) {
                    best_bid_price_level.fill_order(reducing_order);
                } else {
                    best_ask_price_level.fill_order(reducing_order);
                }
                auto order_entry_it = get_order_entry(executing_order.order_id);
                m_order_entries.erase(order_entry_it);
                trades.push_back(Trade{ TradeInfo{
                                          .fill_type = FillType::Full,
                                          .user_id = executing_order.user_id,
                                          .order_id = executing_order.order_id,
                                          .price = executing_order.price,
                                          .quantity = fill_quantity,
                                        },
                                        TradeInfo{ .fill_type = FillType::Partial,
                                                   .user_id = reducing_order.user_id,
                                                   .order_id = reducing_order.order_id,
                                                   .price = reducing_order.price,
                                                   .quantity = fill_quantity } });
            }
        }

        // Any FillOrKill orders that were only partially filled
        // should be deleted from the order book
        if (!m_bids.empty()) {
            auto& best_bid_price_level = m_bids.back();
            auto& bid_ = best_bid_price_level.front();
            if (bid_.order_type == OrderType::FILL_OR_KILL) {
                best_bid_price_level.cancel_order(bid_.order_id);
                auto order_entry_it = get_order_entry(bid_.order_id);
                m_order_entries.erase(order_entry_it);
            }
        }

        if (!m_asks.empty()) {
            auto& best_ask_price_level = m_asks.back();
            auto& ask_ = best_ask_price_level.front();
            if (ask_.order_type == OrderType::FILL_OR_KILL) {
                best_ask_price_level.cancel_order(ask_.order_id);
                auto order_entry_it = get_order_entry(ask_.order_id);
                m_order_entries.erase(order_entry_it);
            }
        }

        return trades;
    }

    void cancel_order(OrderId order_id)
    {
        auto order_entry_iter = get_order_entry(order_id);

        if (order_entry_iter == m_order_entries.end()) {
            throw std::logic_error("Order Id not found");
        }

        OrderEntry& order_entry = *order_entry_iter;
        PriceLevel* price_level_ptr = order_entry.price_level;

        price_level_ptr->cancel_order(order_id);
        auto order_entry_it = get_order_entry(order_id);
        m_order_entries.erase(order_entry_it);
    }

    void modify_order(OrderId order_id, Price new_price, Quantity quantity)
    {
        auto order_entry_iter = get_order_entry(order_id);

        if (order_entry_iter == m_order_entries.end()) {
            throw std::logic_error("Order Id not found");
        }

        OrderEntry& order_entry = *order_entry_iter;
        PriceLevel* price_level_ptr = order_entry.price_level;

        price_level_ptr->cancel_order(order_id);
        auto order_entry_it = get_order_entry(order_id);
        m_order_entries.erase(order_entry_it);

        // if(price_level_ptr->get_price() == new_price){
        //     price_level_ptr->modify_order(order_id, quantity);
        // }
        // else
        //{
        Order old_order = price_level_ptr->get_order(order_id);
        add_order(Order{
          .order_type = old_order.order_type,
          .order_id = generate_order_id(),
          .user_id = old_order.user_id,
          .side = old_order.side,
          .symbol = old_order.symbol,
          .price = new_price,
          .initial_quantity = quantity,
          .remaining_quantity = quantity,
        });

        //}
    }

    Symbol& get_symbol() { return m_symbol; }

  private:
    Symbol m_symbol;
    // Buy orders sorted by price (highest first)
    std::vector<PriceLevel> m_bids;
    // Sell orders sorted by price (lowest first)
    std::vector<PriceLevel> m_asks;
    // All orders sorted by OrderId
    std::deque<OrderEntry> m_order_entries;
};

using OrderBookPtr = OrderBook*;
} // namespace dev

#endif
