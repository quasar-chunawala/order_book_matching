#include "market_data_manager.h"

namespace dev {
// Constructors
MarketDataManager::MarketDataManager()
  : m_order_books{}
{
}

// Getters
std::unordered_map<size_t, OrderBook>&
MarketDataManager::get_order_books()
{
    return m_order_books;
}

auto
MarketDataManager::get_order_book_iter(std::string_view symbol_name)
{
    if (m_order_books.size() == 0) {
        return m_order_books.end();
    }
    auto loc = m_order_books.find(std::hash<std::string_view>{}(symbol_name));

    return loc;
}
OrderBook&
MarketDataManager::get_order_book(std::string_view symbol_name)
{
    auto loc = get_order_book_iter(symbol_name);

    if (loc == m_order_books.end())
        throw std::logic_error("Order book not found for the user-supplied symbol");

    return loc->second;
}

/**
 * @brief Gets the order corresponding to the user-supplied
 * @a order_id in constant-time. Performs error-bounds checking.
 */
Order&
MarketDataManager::get_order(OrderId order_id)
{
    OrderBook& order_book = get_order_book(order_id.symbol_name);
    return order_book.get_order(order_id);
}

// Modifiers
void
MarketDataManager::add_order(OrderType order_type,
                             UserId user_id,
                             Side side,
                             SymbolName symbol_name,
                             Price price,
                             Quantity quantity)
{
    auto it = get_order_book_iter(symbol_name);

    // Order book validation
    if (it == m_order_books.end()) {
        add_order_book(symbol_name);
        it = get_order_book_iter(symbol_name);
    }

    OrderBook& order_book = it->second;
    return order_book.add_order(order_type, user_id, side, symbol_name, price, quantity);
}

void
MarketDataManager::modify_order(OrderId order_id, Price new_price, Quantity new_quantity)
{
    OrderBook& order_book = get_order_book(order_id.symbol_name);
    order_book.modify_order(order_id, new_price, new_quantity);
}

void
MarketDataManager::cancel_order(OrderId order_id)
{
    OrderBook& order_book = get_order_book(order_id.symbol_name);
    order_book.cancel_order(order_id);
}

void
MarketDataManager::add_order_book(std::string_view symbol_name)
{
    m_order_books[std::hash<std::string_view>{}(symbol_name)] = OrderBook{};
}
void
MarketDataManager::delete_order_book(std::string_view symbol_name)
{
    m_order_books.erase(std::hash<std::string_view>{}(symbol_name));
}
}
