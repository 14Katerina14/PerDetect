#pragma once

#include <cstddef>
#include <string_view>

namespace securezone::auth {

inline constexpr std::size_t MinimumPasswordLength = 12;
inline constexpr std::size_t MaximumPasswordLength = 256;

inline bool isPasswordLengthAllowed(std::string_view password) {
    return password.size() >= MinimumPasswordLength
        && password.size() <= MaximumPasswordLength;
}

}
