#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"
#include "securezone/auth/PasswordPolicy.h"

#include <iostream>
#include <string>

int main(int argc, char**) {
    if (argc != 1) {
        std::cerr << "Usage: provide the password on standard input; command-line arguments are rejected.\n";
        return 2;
    }

    std::string password;
    if (!std::getline(std::cin, password)
        || !securezone::auth::isPasswordLengthAllowed(password)) {
        std::cerr << "Password must contain between 12 and 256 characters.\n";
        return 2;
    }

    try {
        std::cout
            << securezone::infrastructure::auth::hashPasswordArgon2id(password)
            << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Password hashing failed: " << error.what() << '\n';
        return 1;
    }
}
