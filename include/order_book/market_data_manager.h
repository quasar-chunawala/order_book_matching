#ifndef MARKET_DATA_MANAGER_H
#define MARKET_DATA_MANAGER_H

#include "order.h"
#include "order_book.h"
#include "price_level.h"
#include "symbol.h"
#include <cassert>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace dev {
/**
 * @brief The MarketDataManager is an orchestrator that manages symbols,
 * order books and orders.
 */
class MarketDataManager
{
  public:
    MarketDataManager() = default;

    void add_order_book(Symbol symbol)
    {
        if (0 <= symbol.m_symbol_id && symbol.m_symbol_id < m_order_books.size()) {
            throw std::logic_error(
              "The order book for this ticker symbol already exists!");
        } else {
            auto loc =
              std::lower_bound(m_order_books.begin(),
                               m_order_books.end(),
                               symbol.m_symbol_name,
                               [](OrderBook& order_book, std::string value) {
                                   return order_book.get_symbol().m_symbol_name < value;
                               });

            m_order_books.insert(loc, OrderBook(symbol, m_order_entries));
        }
    }

    void delete_order_book(Symbol symbol)
    {
        if (0 <= symbol.m_symbol_id && symbol.m_symbol_id < m_order_books.size()) {
            auto loc =
              std::lower_bound(m_order_books.begin(),
                               m_order_books.end(),
                               symbol.m_symbol_name,
                               [](OrderBook& order_book, std::string value) {
                                   return order_book.get_symbol().m_symbol_name < value;
                               });

            m_order_books.erase(loc);
        } else {
            throw std::logic_error(
              "The order book for this ticker symbol was not found!");
        }
    }

    auto get_order_book_iter(std::string symbol_name)
    {
        if (m_order_books.size() == 0) {
            return m_order_books.end();
        }

        auto loc =
          std::lower_bound(m_order_books.begin(),
                           m_order_books.end(),
                           symbol_name,
                           [](OrderBook& order_book, std::string value) {
                               return order_book.get_symbol().m_symbol_name < value;
                           });

        if (loc != m_order_books.end()) {
            if (loc->get_symbol().m_symbol_name != symbol_name) {
                return m_order_books.end();
            }
        }

        return loc;
    }

    OrderBook& get_order_book(std::string symbol_name)
    {
        if (m_order_books.size() == 0) {
            throw std::logic_error("No order books found!");
        }
        auto loc = get_order_book_iter(symbol_name);

        if (loc == m_order_books.end())
            throw std::logic_error("Order book not found for the user-supplied symbol");

        return *loc;
    }

    Trades add_order(const Order& order)
    {
        OrderBooks::iterator it = get_order_book_iter(order.symbol.m_symbol_name);
        // Order book validation
        if (it == m_order_books.end()) {
            add_order_book(order.symbol);
            it = get_order_book_iter(order.symbol.m_symbol_name);
        }

        return it->add_order(order);
    }

    void cancel_order(OrderId order_id)
    {
        PriceLevel& price_level = get_price_level(order_id);
        price_level.cancel_order(order_id);
    }

    auto get_order_entry_iter(OrderId order_id)
    {
        std::deque<OrderEntry>::iterator it{ m_order_entries.begin() };

        if (m_order_entries.size() == 0)
            return m_order_entries.end();

        it = std::lower_bound(m_order_entries.begin(),
                              m_order_entries.end(),
                              order_id,
                              [](OrderEntry& order_entry, OrderId value) {
                                  return order_entry.order_id < value;
                              });

        if (it != m_order_entries.end()) {
            if (it->order_id != order_id)
                return m_order_entries.end();
        }

        return it;
    }

    void modify_order(OrderId order_id, Price new_price, Quantity new_quantity)
    {
        auto it = get_order_entry_iter(order_id);
        OrderEntry order_entry = *it;
        Symbol symbol = order_entry.symbol;
        OrderBook& order_book = get_order_book(it->symbol.m_symbol_name);
        Price price = order_entry.price;
        Order old_order = get_order(order_id);

        if (price == new_price) {
            PriceLevel& price_level = get_price_level(order_id);
            price_level.modify_order(order_id, new_quantity);
            order_book.match();
        } else {
            order_book.cancel_order(order_id);
            order_book.add_order({ .order_type = old_order.order_type,
                                   .order_id = order_book.generate_order_id(),
                                   .user_id = old_order.user_id,
                                   .side = old_order.side,
                                   .symbol = old_order.symbol,
                                   .price = new_price,
                                   .initial_quantity = new_quantity,
                                   .remaining_quantity = new_quantity });
        }
    }

    Order get_order(OrderId order_id)
    {
        std::deque<OrderEntry>::iterator it = get_order_entry_iter(order_id);

        if (it == m_order_entries.end())
            throw std::logic_error("Order with user-supplied order-id not found!");

        OrderEntry order_entry = *it;
        OrderBook& order_book = get_order_book(it->symbol.m_symbol_name);
        Order order = order_book.get_order(order_id);
        return order;
    }

    PriceLevel& get_price_level(OrderId order_id)
    {
        std::deque<OrderEntry>::iterator it = get_order_entry_iter(order_id);

        if (it == m_order_entries.end())
            throw std::logic_error("Order with user-supplied order-id not found!");

        OrderEntry order_entry = *it;
        OrderBook& order_book = get_order_book(it->symbol.m_symbol_name);
        LevelType level_type = it->side == 'B' ? LevelType::BID : LevelType::ASK;
        PriceLevel& price_level = order_book.get_price_level(level_type, it->price);
        return price_level;
    }

  private:
    std::vector<OrderBook> m_order_books;
    // All orders sorted by OrderId
    std::deque<OrderEntry> m_order_entries;
};

}

#endif