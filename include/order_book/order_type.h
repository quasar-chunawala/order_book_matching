#ifndef ORDER_TYPE_H
#define ORDER_TYPE_H

#include <iostream>

namespace dev{
    enum class OrderType{
        MARKET,
        LIMIT,
        //STOP,
        //STOP_LIMIT,
        FILL_OR_KILL,
        //GOOD_TILL_CANCEL
    };
};
#endif