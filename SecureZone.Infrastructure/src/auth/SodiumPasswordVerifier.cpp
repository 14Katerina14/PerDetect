#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"

#include "securezone/auth/PasswordPolicy.h"

#include <sodium.h>

#include <array>
#include <stdexcept>
#include <string>

namespace securezone::infrastructure::auth {
namespace {

void ensureSodiumInitialized() {
    static const bool initialized = sodium_init() >= 0;
    if (!initialized) {
        throw std::runtime_error("libsodium initialization failed.");
    }
}

}

SodiumPasswordVerifier::SodiumPasswordVerifier() {
    ensureSodiumInitialized();
}

bool SodiumPasswordVerifier::verify(
    std::string_view password,
    std::string_view passwordHash
) const {
    if (password.empty() || passwordHash.empty()) {
        return false;
    }

    ensureSodiumInitialized();
    const std::string passwordValue{password};
    const std::string hashValue{passwordHash};
    return crypto_pwhash_str_verify(
        hashValue.c_str(),
        passwordValue.data(),
        static_cast<unsigned long long>(passwordValue.size())
    ) == 0;
}

std::string hashPasswordArgon2id(std::string_view password) {
    if (!securezone::auth::isPasswordLengthAllowed(password)) {
        throw std::invalid_argument("Password must contain between 12 and 256 characters.");
    }

    ensureSodiumInitialized();
    std::array<char, crypto_pwhash_STRBYTES> encoded{};
    const std::string passwordValue{password};
    if (crypto_pwhash_str(
            encoded.data(),
            passwordValue.data(),
            static_cast<unsigned long long>(passwordValue.size()),
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE
        ) != 0) {
        throw std::runtime_error("Argon2id password hashing failed.");
    }

    return encoded.data();
}

}
