#ifndef ORDERID_H
#define ORDERID_H

#include <cstdint>
#include <string>

namespace dev {
using SymbolName = char[4];

struct OrderId
{
    char symbol_name[4];
    uint32_t seq_num;
};
}

#endif