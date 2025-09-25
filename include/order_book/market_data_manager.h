#ifndef MARKET_DATA_MANAGER_H
#define MARKET_DATA_MANAGER_H

#include "order_type.h"
#include "usings.h"
#include <cassert>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace dev {

class OrderBook;
struct Order;
struct OrderId;
/**
 * @brief The MarketDataManager is an orchestrator that manages symbols,
 * order books and orders.
 */
class MarketDataManager
{
  public:
    // Constructors
    MarketDataManager();
    MarketDataManager(const MarketDataManager&) = delete;
    MarketDataManager& operator=(const MarketDataManager&) = delete;
    MarketDataManager(MarketDataManager&&) = delete;
    MarketDataManager& operator=(MarketDataManager&&) = delete;

    // Public API
    // Getters
    OrderBook& get_order_book(std::string_view symbol_name);
    Order& get_order(OrderId order_id);

    // Modifiers
    void add_order(OrderType order_type,
                   UserId user_id,
                   Side side,
                   std::string_view symbol_name,
                   Price price,
                   Quantity quantity);
    void modify_order(OrderId order_id, Price new_price, Quantity new_quantity);
    void cancel_order(OrderId order_id);

  private:
    // Constant-time access to orderbook
    std::unordered_map<size_t, OrderBook> m_order_books;

    // Getters
    std::unordered_map<size_t, OrderBook>& get_order_books();
    auto get_order_book_iter(std::string_view symbol_name);

    // Modifiers
    void add_order_book(std::string_view symbol_name);
    void delete_order_book(std::string_view symbol_name);
};
// Ingestion Thread that reads data from the socket
// OrderBook -> PruneDayOrders
// Main thread - MarketDataManager
// Enqueue trades filled into -> MPMC Lock-free queue -> Publisher thread Market
// dissemination
}
#endif
