#pragma once

#include <string_view>

namespace securezone::auth {

class IPasswordVerifier {
public:
    virtual ~IPasswordVerifier() = default;

    virtual bool verify(
        std::string_view password,
        std::string_view passwordHash
    ) const = 0;
};

}
