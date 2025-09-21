#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "formatter.h"
#include "order.h"
#include "order_type.h"
#include "price_level.h"
#include "symbol.h"
#include "trade.h"
#include "trade_info.h"
#include <algorithm>
#include <chrono>
#include <deque>
#include <format>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
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
    Symbol symbol;
    Price price;
    Side side;
};

class OrderBook
{

    auto find_insert_location(PriceLevels& price_levels,
                              LevelType level_type,
                              Price price)
    {
        // Empty queue
        if (price_levels.size() == 0)
            return price_levels.end();

        PriceLevels::iterator pos;

        if (level_type == LevelType::BID) {
            // m_bids.back() is the highest bid
            pos = std::lower_bound(price_levels.begin(),
                                   price_levels.end(),
                                   price,
                                   [](PriceLevel& price_level, Price value) {
                                       return price_level.get_price() < value;
                                   });
        } else {
            // m_asks.back() is the lowest ask
            pos = std::lower_bound(price_levels.begin(),
                                   price_levels.end(),
                                   price,
                                   [](PriceLevel& price_level, Price value) {
                                       return price_level.get_price() > value;
                                   });
        }

        return pos;
    }

  public:
    OrderId generate_order_id()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
          .count();
    }

    OrderBook(Symbol symbol, std::deque<OrderEntry>& order_entries)
      : m_symbol{ symbol }
      , m_order_entries{ order_entries }
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
      : m_symbol{ std::exchange(other.m_symbol, {}) }
      , m_bids{ std::exchange(other.m_bids, {}) }
      , m_asks{ std::exchange(other.m_asks, {}) }
      , m_order_entries{ other.m_order_entries }
    {
    }

    OrderBook& operator=(OrderBook&& other)
    {
        OrderBook(std::move(other)).swap(*this);
        return *this;
    }

    auto get_price_level_iter(LevelType level_type, Price price)
    {
        PriceLevels& price_levels = level_type == LevelType::BID ? m_bids : m_asks;

        PriceLevels::iterator pos{};

        if (level_type == LevelType::BID) {
            // m_bids.back() is the highest bid
            pos = std::lower_bound(price_levels.begin(),
                                   price_levels.end(),
                                   price,
                                   [](PriceLevel& price_level, Price value) {
                                       return price_level.get_price() < value;
                                   });
        } else {
            // m_asks.back() is the lowest ask

            pos = std::lower_bound(price_levels.begin(),
                                   price_levels.end(),
                                   price,
                                   [](PriceLevel& price_level, Price value) {
                                       return price_level.get_price() > value;
                                   });
        }

        if (pos != price_levels.end())
            if (pos->get_price() != price)
                return price_levels.end();

        return pos;
    }

    PriceLevels& get_price_levels(LevelType level_type)
    {
        if (level_type == LevelType::BID)
            return m_bids;
        else
            return m_asks;
    }

    /**
     * @brief Adds a price level
     */
    void add_price_level(LevelType type,
                         Price price,
                         const OrderQueue& order_queue = std::deque<Order>{})
    {
        PriceLevels& price_levels = type == LevelType::BID ? m_bids : m_asks;
        PriceLevels::iterator pos = find_insert_location(price_levels, type, price);
        price_levels.insert(pos, PriceLevel{ type, price, order_queue });
    }

    /**
     * @brief Deletes a price level
     */
    void delete_price_level(LevelType type, Price price)
    {
        PriceLevels& price_levels = type == LevelType::BID ? m_bids : m_asks;
        PriceLevels::iterator pos = get_price_level_iter(type, price);
        price_levels.erase(pos);
    }

    PriceLevel& get_bid_price_level(Price price)
    {
        return get_price_level(LevelType::BID, price);
    }

    PriceLevel& get_ask_price_level(Price price)
    {
        return get_price_level(LevelType::ASK, price);
    }

    PriceLevel& get_price_level(LevelType level_type, Price price)
    {
        PriceLevels::iterator pos = get_price_level_iter(level_type, price);
        return *pos;
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
    Trades add_order(Order order)
    {
        Side side = order.side;
        Price price = order.price;

        // Handling for MARKET orders
        if (order.order_type == OrderType::MARKET) {
            if (order.side == 'B' and !m_asks.empty()) {
                PriceLevel& worst_ask = m_asks.front();
                // Convert to a limit order with worst ask price
                order.order_type = OrderType::LIMIT;
                order.price = worst_ask.get_price();
            } else if (order.side == 'S' and !m_bids.empty()) {
                PriceLevel& worst_bid = m_bids.front();
                // Convert to a limit order with worst bid price
                order.order_type = OrderType::LIMIT;
                order.price = worst_bid.get_price();
            } else {
                return {};
            }
        }
        LevelType level_type = side == 'B' ? LevelType::BID : LevelType::ASK;
        PriceLevels& price_levels = level_type == LevelType::BID ? m_bids : m_asks;
        PriceLevels::iterator it = get_price_level_iter(level_type, price);

        if (it == price_levels.end()) {
            add_price_level(level_type, price);
            it = get_price_level_iter(level_type, price);
        }

        PriceLevel& price_level = *it;

        if (order.order_type == OrderType::FILL_AND_KILL &&
            !is_match_possible(order.side, order.price))
            return {};

        price_level.add_order(order);
        m_order_entries.push_back(OrderEntry{
          .order_id = order.order_id,
          .symbol = order.symbol,
          .price = order.price,
          .side = side,
        });
        return match();
    }

    const PriceLevel& get_best_bid() const { return m_bids.back(); }

    PriceLevel& get_best_bid() { return m_bids.back(); }

    const PriceLevel& get_best_ask() const { return m_asks.back(); }

    PriceLevel& get_best_ask() { return m_asks.back(); }

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

                OrderId executing_order_id = executing_order.order_id;
                OrderId reducing_order_id = reducing_order.order_id;
                FillType reducing_order_fill_type =
                  (fill_quantity == reducing_order.remaining_quantity &&
                   reducing_order.initial_quantity == reducing_order.remaining_quantity)
                    ? FillType::Full
                    : FillType::Partial;

                if (executing_order.order_id == bid_.order_id) {
                    best_bid_price_level.fill_order(reducing_order);

                    if (reducing_order.remaining_quantity == 0) {
                        best_ask_price_level.pop_front();

                        auto it = get_order_entry(executing_order_id);
                        m_order_entries.erase(it);
                    }
                } else {
                    best_ask_price_level.fill_order(reducing_order);
                    if (reducing_order.remaining_quantity == 0) {
                        best_bid_price_level.pop_front();

                        auto it = get_order_entry(reducing_order_id);
                        m_order_entries.erase(it);
                    }
                }

                auto order_entry_it = get_order_entry(executing_order_id);
                m_order_entries.erase(order_entry_it);

                TradeInfo executing_order_fill_info = TradeInfo{
                    .fill_type = FillType::Full,
                    .user_id = executing_order.user_id,
                    .order_id = executing_order.order_id,
                    .symbol = executing_order.symbol,
                    .price = executing_order.price,
                    .quantity = fill_quantity,
                };

                TradeInfo reducing_order_fill_info =
                  TradeInfo{ .fill_type = reducing_order_fill_type,
                             .user_id = reducing_order.user_id,
                             .order_id = reducing_order.order_id,
                             .symbol = reducing_order.symbol,
                             .price = reducing_order.price,
                             .quantity = fill_quantity };

                std::string out1_ =
                  std::format("executing_order_fill_info={}", executing_order_fill_info);
                std::string out2_ =
                  std::format("reducing_order_fill_info={}", reducing_order_fill_info);

                std::cout << "\n" << out1_;
                std::cout << "\n" << out2_;
                std::cout << "\n";

                trades.push_back(
                  Trade{ executing_order_fill_info, reducing_order_fill_info });
            }

            if (best_bid_price_level.get_count() == 0) {
                m_bids.pop_back();
            }

            if (best_ask_price_level.get_count() == 0) {
                m_asks.pop_back();
            }
        }

        // Any FillAndKill orders that were only partially filled
        // should be deleted from the order book
        if (!m_bids.empty()) {
            auto& best_bid_price_level = m_bids.back();
            auto& bid_ = best_bid_price_level.front();
            if (bid_.order_type == OrderType::FILL_AND_KILL) {
                best_bid_price_level.cancel_order(bid_.order_id);
                auto order_entry_it = get_order_entry(bid_.order_id);
                m_order_entries.erase(order_entry_it);
            }
        }

        if (!m_asks.empty()) {
            auto& best_ask_price_level = m_asks.back();
            auto& ask_ = best_ask_price_level.front();
            if (ask_.order_type == OrderType::FILL_AND_KILL) {
                best_ask_price_level.cancel_order(ask_.order_id);
                auto order_entry_it = get_order_entry(ask_.order_id);
                m_order_entries.erase(order_entry_it);
            }
        }

        return trades;
    }

    PriceLevel& order_id_to_price_level(OrderId order_id)
    {
        auto order_entry_iter = get_order_entry(order_id);

        if (order_entry_iter == m_order_entries.end()) {
            throw std::logic_error("Order Id not found");
        }

        OrderEntry order_entry = *order_entry_iter;
        LevelType level_type = order_entry.side == 'B' ? LevelType::BID : LevelType::ASK;
        PriceLevel& price_level = get_price_level(level_type, order_entry.price);
        return price_level;
    }

    void cancel_order(OrderId order_id)
    {
        PriceLevel& price_level = order_id_to_price_level(order_id);

        price_level.cancel_order(order_id);
        auto order_entry_it = get_order_entry(order_id);
        m_order_entries.erase(order_entry_it);
    }

    Order get_order(OrderId order_id)
    {
        PriceLevel& price_level = order_id_to_price_level(order_id);
        Order order = price_level.get_order(order_id);
        return order;
    }

    void modify_order(OrderId order_id, Price new_price, Quantity quantity)
    {
        PriceLevel& price_level = order_id_to_price_level(order_id);
        Order old_order = price_level.get_order(order_id);

        price_level.cancel_order(order_id);
        auto order_entry_it = get_order_entry(order_id);
        m_order_entries.erase(order_entry_it);

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
    // Reference to vector of order entries (sorted by OrderId)
    std::deque<OrderEntry>& m_order_entries;
};

using OrderBooks = std::vector<OrderBook>;
} // namespace dev

#endif
