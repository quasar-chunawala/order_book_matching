#include "order_book.h"
#include "formatter.h"
#include <cstring>

namespace dev {
OrderBook::OrderBook()
  : m_bids{}
  , m_asks{}
  , m_order_pool{}
  , m_free_list{}
{
    m_order_pool.reserve(10000);
    m_order_pool.push_back(OrderNode{ .order = std::nullopt, .prev = 0u, .next = 0u });
}

// Getters
/**
 * @brief Get the order for the user-supplied @a order_id.
 * It has O(1) run-time complexity.
 */
Order&
OrderBook::get_order(OrderId order_id)
{
    if (!order_exists(order_id))
        throw std::logic_error(std::format("OrderId {} does not exist!", order_id));

    return m_order_pool[order_id.seq_num].order.value();
}

/**
 * @brief Get all bids in the order book.
 */
PriceLevels&
OrderBook::get_bids()
{
    return m_bids;
}

/**
 * @brief Get all asks in the order book.
 */
PriceLevels&
OrderBook::get_asks()
{
    return m_asks;
}

PriceLevels&
OrderBook::get_price_levels(LevelType level_type)
{
    return level_type == LevelType::BID ? m_bids : m_asks;
}

/**
 * @brief Get the best bid in constant-time.
 */
const PriceLevel&
OrderBook::get_best_bid() const
{
    return m_bids.back();
}

/**
 * @brief Get the best ask in constant-time.
 */
PriceLevel&
OrderBook::get_best_bid()
{
    return m_bids.back();
}

const PriceLevel&
OrderBook::get_best_ask() const
{
    return m_asks.back();
}

PriceLevel&
OrderBook::get_best_ask()
{
    return m_asks.back();
}

/**
 * @brief Perform a binary search
 * and get an iterator to the price level in O(log n) time
 */
auto
OrderBook::get_price_level_iter(LevelType level_type, Price price)
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

/**
 * @brief Get the bid price level for user-supplied @a price.
 */
PriceLevel&
OrderBook::get_bid_price_level(Price price)
{
    auto it = get_price_level_iter(LevelType::BID, price);
    if (it == m_bids.end()) {
        throw std::logic_error(std::format("Price level {} not found!", price));
    }

    return *it;
}

/**
 * @brief Get the ask price level for user-supplied @a price.
 */
PriceLevel&
OrderBook::get_ask_price_level(Price price)
{
    auto it = get_price_level_iter(LevelType::ASK, price);
    if (it == m_asks.end()) {
        throw std::logic_error(std::format("Price level {} not found!", price));
    }

    return *it;
}

/**
 * @brief Get the price level for the user-supplied @a level_type and @a price.
 */
PriceLevel&
OrderBook::get_price_level(LevelType level_type, Price price)
{
    return level_type == LevelType::BID ? get_bid_price_level(price)
                                        : get_ask_price_level(price);
}

/**
 * @brief Get the free-list of indices.
 */
std::deque<SeqNum>&
OrderBook::get_free_list()
{
    return m_free_list;
}

bool
OrderBook::is_match_possible(Side side, Price price)
{
    if (side == 'B') {
        if (m_asks.empty())
            return false;

        if (price < m_asks.back().get_price())
            return false;

        if (m_asks.back().is_empty())
            return false;
    } else {
        if (m_bids.empty())
            return false;

        if (price > m_bids.back().get_price())
            return false;

        if (m_bids.back().is_empty())
            return false;
    }
    return true;
}

bool
OrderBook::order_exists(OrderId order_id)
{
    if (order_id.seq_num == 0 || order_id.seq_num > m_order_pool.size())
        return false;

    OrderNode& order_node = m_order_pool[order_id.seq_num];
    auto& order_opt = order_node.order;
    if (!order_opt.has_value())
        return false;

    return true;
}

// Order management API
/**
 * @brief Add an order to the order_book
 */
void
OrderBook::add_order(OrderType order_type,
                     UserId user_id,
                     Side side,
                     std::string_view symbol_name,
                     Price price,
                     Quantity quantity)
{

    // Handling for MARKET orders
    if (order_type == OrderType::MARKET) {
        if (side == 'B' and !m_asks.empty()) {
            PriceLevel& worst_ask = m_asks.front();
            // Convert to a limit order with worst ask price
            order_type = OrderType::LIMIT;
            price = std::numeric_limits<Price>::max();
        } else if (side == 'S' and !m_bids.empty()) {
            PriceLevel& worst_bid = m_bids.front();
            // Convert to a limit order with worst bid price
            order_type = OrderType::LIMIT;
            price = std::numeric_limits<Price>::min();
        } else {
            return;
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
    OrderId order_id = generate_order_id(symbol_name);

    if (order_type == OrderType::FILL_AND_KILL && !is_match_possible(side, price))
        return;

    price_level.push_back(Order{ .order_type = order_type,
                                 .order_id = order_id,
                                 .user_id = user_id,
                                 .side = side,
                                 .price = price,
                                 .initial_quantity = quantity,
                                 .remaining_quantity = quantity });
    match();
}

void
OrderBook::modify_order(OrderId order_id, Price new_price, Quantity new_quantity)
{
    Order& old_order = get_order(order_id);
    OrderType order_type = old_order.order_type;
    UserId user_id = old_order.user_id;
    Side side = old_order.side;
    LevelType level_type = old_order.side == 'B' ? LevelType::BID : LevelType::ASK;
    PriceLevel& price_level = get_price_level(level_type, old_order.price);

    if (old_order.price == new_price) {
        old_order.initial_quantity = new_quantity;
        old_order.remaining_quantity = new_quantity;
        return;
    }

    cancel_order(order_id);

    add_order(order_type, user_id, side, order_id.symbol_name, new_price, new_quantity);
}

void
OrderBook::cancel_order(OrderId order_id)
{
    if (!order_exists(order_id))
        throw std::logic_error(std::format("OrderId {} does not exist!", order_id));

    OrderNode& order_node = m_order_pool[order_id.seq_num];
    if (order_node.prev = 0 || order_node.next == 0) {
        // if(!order_node.order.has_value())
    }
    OrderNode& prev_node = m_order_pool[order_node.prev];
    OrderNode& next_node = m_order_pool[order_node.next];
    prev_node.next = order_node.next;
    next_node.prev = order_node.prev;
    order_node.prev = 0;
    order_node.next = 0;
    m_free_list.push_back(order_id.seq_num);
    order_node.order = std::nullopt;
}

/**
 * @brief The global match method attempts match orders
 * in priority of (price, arrival time).
 */
void
OrderBook::match()
{
    Trades trades{};

    while (true) {
        if (m_bids.empty() || m_asks.empty())
            break;

        PriceLevel& best_bid_price_level = m_bids.back();
        PriceLevel& best_ask_price_level = m_asks.back();

        if (best_bid_price_level.get_price() < best_ask_price_level.get_price())
            break;

        while (!best_bid_price_level.is_empty() && !best_ask_price_level.is_empty()) {
            auto& bid_ = best_bid_price_level.front();
            auto& ask_ = best_ask_price_level.front();

            Quantity fill_quantity =
              std::min(bid_.remaining_quantity, ask_.remaining_quantity);

            auto& executing_order = bid_;
            auto& reducing_order = ask_;

            if (executing_order.remaining_quantity > reducing_order.remaining_quantity) {
                std::swap(executing_order, reducing_order);
            }

            OrderId executing_order_id = executing_order.order_id;
            OrderId reducing_order_id = reducing_order.order_id;
            FillType reducing_order_fill_type =
              (fill_quantity == reducing_order.remaining_quantity &&
               reducing_order.initial_quantity == reducing_order.remaining_quantity)
                ? FillType::Full
                : FillType::Partial;

            if (executing_order.order_id.seq_num == bid_.order_id.seq_num) {
                best_bid_price_level.fill_order(reducing_order);

                if (reducing_order.remaining_quantity == 0) {
                    best_ask_price_level.pop_front();
                }
            } else {
                best_ask_price_level.fill_order(reducing_order);
                if (reducing_order.remaining_quantity == 0) {
                    best_bid_price_level.pop_front();
                }
            }

            TradeInfo executing_order_fill_info = TradeInfo{
                .fill_type = FillType::Full,
                .user_id = executing_order.user_id,
                .order_id = executing_order.order_id,
                .price = executing_order.price,
                .quantity = fill_quantity,
            };

            TradeInfo reducing_order_fill_info =
              TradeInfo{ .fill_type = reducing_order_fill_type,
                         .user_id = reducing_order.user_id,
                         .order_id = reducing_order.order_id,
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

        if (best_bid_price_level.is_empty()) {
            m_bids.pop_back();
        }

        if (best_ask_price_level.is_empty()) {
            m_asks.pop_back();
        }
    }

    // Any FillAndKill orders that were only partially filled
    // should be deleted from the order book
    if (!m_bids.empty()) {
        auto& best_bid_price_level = m_bids.back();
        auto& bid_ = best_bid_price_level.front();
        if (bid_.order_type == OrderType::FILL_AND_KILL) {
            cancel_order(bid_.order_id);
        }
    }

    if (!m_asks.empty()) {
        auto& best_ask_price_level = m_asks.back();
        auto& ask_ = best_ask_price_level.front();
        if (ask_.order_type == OrderType::FILL_AND_KILL) {
            cancel_order(ask_.order_id);
        }
    }
}

void
OrderBook::add_price_level(LevelType level_type, Price price)
{
    PriceLevels& price_levels = level_type == LevelType::BID ? m_bids : m_asks;
    PriceLevels::iterator pos = find_insert_location(price_levels, level_type, price);
    price_levels.insert(pos, PriceLevel{ level_type, price, *this });
}

void
OrderBook::delete_price_level(LevelType level_type, Price price)
{
    PriceLevels& price_levels = level_type == LevelType::BID ? m_bids : m_asks;
    PriceLevels::iterator pos = get_price_level_iter(level_type, price);
    price_levels.erase(pos);
}

SeqNum
OrderBook::get_next_seq_num()
{
    SeqNum next_seq_num{};
    if (!m_free_list.empty()) {
        next_seq_num = m_free_list.front();
        m_free_list.pop_front();
    } else {
        next_seq_num = m_order_pool.size();
    }
    return next_seq_num;
}

OrderId
OrderBook::generate_order_id(std::string_view symbol_name)
{
    OrderId order_id{ .symbol_name = "", .seq_num = get_next_seq_num() };
    strcpy(order_id.symbol_name, std::string(symbol_name).c_str());
    return order_id;
}

/**
 * @brief Helper function to find the location in vector<PriceLevel>
 * where the user-supplied @a price must be inserted.
 */
PriceLevels::iterator
OrderBook::find_insert_location(PriceLevels& price_levels,
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

}