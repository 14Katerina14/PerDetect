#pragma once

#include "securezone/auth/IPasswordVerifier.h"

#include <string>
#include <string_view>

namespace securezone::infrastructure::auth {

class SodiumPasswordVerifier final : public securezone::auth::IPasswordVerifier {
public:
    SodiumPasswordVerifier();

    bool verify(
        std::string_view password,
        std::string_view passwordHash
    ) const override;
};

std::string hashPasswordArgon2id(std::string_view password);

}
