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

            m_order_books.insert(loc, OrderBook(symbol));
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

    Trades add_market_order(const Order& market_order)
    {
        OrderBookPtr order_book = get_order_book(market_order.symbol.m_symbol_name);
        // Order book validation
        if (!order_book) {
            add_order_book(market_order.symbol);
            order_book = get_order_book(market_order.symbol.m_symbol_name);
        }

        return order_book->add_order(market_order);
    }

    OrderBookPtr get_order_book(std::string symbol_name)
    {
        auto loc =
          std::lower_bound(m_order_books.begin(),
                           m_order_books.end(),
                           symbol_name,
                           [](OrderBook& order_book, std::string value) {
                               return order_book.get_symbol().m_symbol_name < value;
                           });

        if (loc == m_order_books.end())
            return nullptr;

        return std::addressof(*loc);
    }

  private:
    std::vector<OrderBook> m_order_books;
};
}

#endif