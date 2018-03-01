#pragma once
#include "memory.h"
#include <boost/asio.hpp>

namespace mdk
{
    template<typename T> 
    inline auto to_asio_buffer(T& value)
    {
        return boost::asio::buffer(to_binary(value));
    }
}
