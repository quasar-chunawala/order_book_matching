#include "price_level.h"
#include "order_book.h"
#include "order_node.h"

namespace dev {
using OrderPool = std::vector<OrderNode>;

PriceLevel::PriceLevel(LevelType type, Price price, OrderBook& order_book)
  : m_level_type{ type }
  , m_price{ price }
  , m_first_seq_num{ 0u }
  , m_last_seq_num{ 0u }
  , m_order_book{ order_book }
{
}
void
PriceLevel::swap(PriceLevel& other)
{
    std::swap(m_level_type, other.m_level_type);
    std::swap(m_price, other.m_price);
    std::swap(m_first_seq_num, other.m_first_seq_num);
    std::swap(m_last_seq_num, other.m_last_seq_num);
}
PriceLevel::PriceLevel(PriceLevel&& other)
  : m_level_type{ std::exchange(other.m_level_type, LevelType::BID) }
  , m_price{ std::exchange(other.m_price, 0u) }
  , m_first_seq_num{ std::exchange(other.m_first_seq_num, 0u) }
  , m_last_seq_num{ std::exchange(other.m_last_seq_num, 0u) }
  , m_order_book{ other.m_order_book }
{
}

PriceLevel&
PriceLevel::operator=(PriceLevel&& other)
{
    PriceLevel(std::move(other)).swap(*this);
    return *this;
}

LevelType
PriceLevel::get_level_type() const
{
    return m_level_type;
}
Price
PriceLevel::get_price() const
{
    return m_price;
}
SeqNum
PriceLevel::get_first_seq_num()
{
    return m_first_seq_num;
}
SeqNum
PriceLevel::get_last_seq_num()
{
    return m_last_seq_num;
}
Order&
PriceLevel::front()
{
    return m_order_book.m_order_pool[m_first_seq_num].order.value();
}

Order&
PriceLevel::back()
{
    return m_order_book.m_order_pool[m_last_seq_num].order.value();
}

void
PriceLevel::on_empty_helper(Order order)
{
    m_first_seq_num = order.order_id.seq_num;
    m_last_seq_num = m_first_seq_num;
    m_order_book.m_order_pool[order.order_id.seq_num] = {
        .order = order,
        .prev = 0,
        .next = 0,
    };
}

void
PriceLevel::push_front(Order order)
{
    if (is_empty()) {
        on_empty_helper(order);
        return;
    }
    OrderPool& order_pool = m_order_book.m_order_pool;
    OrderNode& front = order_pool[m_first_seq_num];
    SeqNum new_node_seq_num = order.order_id.seq_num;
    if (new_node_seq_num > order_pool.size()) {
        order_pool.push_back({
          .order = order,
          .prev = 0,
          .next = m_first_seq_num,
        });
    } else {
        order_pool[new_node_seq_num] = {
            .order = order,
            .prev = 0,
            .next = m_first_seq_num,
        };
    }
    front.prev = new_node_seq_num;
    m_first_seq_num = new_node_seq_num;
}

void
PriceLevel::pop_front()
{
    OrderPool& order_pool = m_order_book.m_order_pool;
    FreeList m_free_list = m_order_book.m_free_list;
    OrderNode& old_head = order_pool[m_first_seq_num];
    if (old_head.next == 0) {
        m_free_list.push_back(m_first_seq_num);
        m_first_seq_num = 0;
        m_last_seq_num = 0;
    } else {
        OrderNode& new_head = order_pool[old_head.next];
        m_free_list.push_back(m_first_seq_num);
        m_first_seq_num = old_head.next;
    }

    old_head.order = std::nullopt;
    old_head.prev = 0;
    old_head.next = 0;
}

void
PriceLevel::push_back(Order order)
{
    if (is_empty()) {
        on_empty_helper(order);
        return;
    }
    OrderPool& order_pool = m_order_book.m_order_pool;
    OrderNode& back = order_pool[m_last_seq_num];
    SeqNum new_node_seq_num = order.order_id.seq_num;
    if (new_node_seq_num > order_pool.size()) {
        order_pool.push_back({
          .order = order,
          .prev = m_last_seq_num,
          .next = 0,
        });
    } else {
        order_pool[new_node_seq_num] = {
            .order = order,
            .prev = m_last_seq_num,
            .next = 0,
        };
    }
    back.next = new_node_seq_num;
    m_last_seq_num = new_node_seq_num;
}
void
PriceLevel::pop_back()
{
    OrderPool& order_pool = m_order_book.m_order_pool;
    FreeList& free_list = m_order_book.m_free_list;
    OrderNode& old_tail = order_pool[m_last_seq_num];
    free_list.push_back(m_last_seq_num);
    if (old_tail.prev == 0) {
        m_first_seq_num = 0;
        m_last_seq_num = 0;
    } else {
        OrderNode& new_tail = order_pool[old_tail.prev];
        m_last_seq_num = old_tail.prev;
        new_tail.next = 0;
    }

    old_tail.order = std::nullopt;
    old_tail.prev = 0;
    old_tail.next = 0;
}

void
PriceLevel::fill_order(Order order)
{
    Order& executing_order = front();
    Order& reducing_order = order;
    Quantity fill_quantity =
      std::min(executing_order.remaining_quantity, reducing_order.remaining_quantity);
    executing_order.remaining_quantity -= fill_quantity;
    reducing_order.remaining_quantity -= fill_quantity;
    if (executing_order.remaining_quantity == 0)
        pop_front();
}
bool
PriceLevel::can_fill(Order order)
{
    bool is_price_cond_met =
      order.side == 'B' ? (m_level_type == LevelType::ASK && m_price <= order.price)
                        : (m_level_type == LevelType::BID && m_price >= order.price);
    return is_price_cond_met;
}
bool
PriceLevel::is_empty()
{
    return m_first_seq_num == 0 && m_last_seq_num == 0;
}
}
