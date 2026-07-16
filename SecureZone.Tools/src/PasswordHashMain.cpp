#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"

#include <iostream>
#include <string>

int main(int argc, char**) {
    if (argc != 1) {
        std::cerr << "Usage: provide the password on standard input; command-line arguments are rejected.\n";
        return 2;
    }

    std::string password;
    if (!std::getline(std::cin, password) || password.empty()) {
        std::cerr << "A non-empty password is required on standard input.\n";
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
