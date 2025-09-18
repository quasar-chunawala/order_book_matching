#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstdint>
#include <string>
namespace dev{
    struct Symbol{
        uint64_t m_symbol_id;
        std::string m_symbol_name;
    };
}
#endif