#pragma once

namespace mdk
{
    template<typename T> 
    inline auto& to_binary(T& value)
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a POD type.");
        return reinterpret_cast<char(&)[sizeof(T)]>(value);
    }
}
