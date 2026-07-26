#include "securezone/auth/PasswordPolicy.h"
#include "securezone/domain/AppUser.h"
#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"
#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/MongoDbSettingsProvider.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>
#include <sodium.h>

#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

struct Arguments {
    std::string userId;
    std::string username;
    std::string role;
    std::string employeeId;
};

std::optional<Arguments> parseArguments(int argc, char** argv) {
    Arguments result;
    for (int index = 1; index < argc; index += 2) {
        if (index + 1 >= argc) return std::nullopt;
        const std::string_view name{argv[index]};
        const std::string value{argv[index + 1]};
        if (name == "--user-id") result.userId = value;
        else if (name == "--username") result.username = value;
        else if (name == "--role") result.role = value;
        else if (name == "--employee-id") result.employeeId = value;
        else return std::nullopt;
    }

    if (result.userId.empty() || result.username.empty() || result.role.empty()) {
        return std::nullopt;
    }
    return result;
}

void printUsage() {
    std::cerr
        << "Usage: SecureZone.ProvisionUser --user-id ID --username NAME "
        << "--role worker|scanner|manager|admin [--employee-id ID]\n"
        << "Provide the password on standard input.\n";
}

}

int main(int argc, char** argv) {
    const auto arguments = parseArguments(argc, argv);
    if (!arguments.has_value()) {
        printUsage();
        return 2;
    }

    const auto role = securezone::domain::appUserRoleFromString(arguments->role);
    if (!role.has_value()
        || (*role == securezone::domain::AppUserRole::Worker && arguments->employeeId.empty())) {
        printUsage();
        return 2;
    }

    std::string password;
    if (!std::getline(std::cin, password)) {
        std::cerr << "Password must contain between 12 and 256 characters.\n";
        return 2;
    }
    if (!password.empty() && password.back() == '\r') {
        password.pop_back();
    }
    if (!securezone::auth::isPasswordLengthAllowed(password)) {
        std::cerr << "Password must contain between 12 and 256 characters.\n";
        return 2;
    }

    try {
        const auto passwordHash =
            securezone::infrastructure::auth::hashPasswordArgon2id(password);
        sodium_memzero(password.data(), password.size());

        auto settings = securezone::infrastructure::mongodb::loadMongoDbSettingsFromEnvironment();
        securezone::infrastructure::mongodb::MongoDbClient client{settings};
        auto appUsers = client.database()["app_users"];

        bsoncxx::builder::basic::document filter;
        filter.append(bsoncxx::builder::basic::kvp("userId", arguments->userId));

        bsoncxx::builder::basic::document fields;
        fields.append(
            bsoncxx::builder::basic::kvp("userId", arguments->userId),
            bsoncxx::builder::basic::kvp("username", arguments->username),
            bsoncxx::builder::basic::kvp("employeeId", arguments->employeeId),
            bsoncxx::builder::basic::kvp("passwordHash", passwordHash),
            bsoncxx::builder::basic::kvp("role", arguments->role),
            bsoncxx::builder::basic::kvp("status", "active")
        );

        bsoncxx::builder::basic::document update;
        update.append(bsoncxx::builder::basic::kvp("$set", fields.extract()));
        mongocxx::options::update options;
        options.upsert(true);
        const auto result = appUsers.update_one(filter.view(), update.view(), options);
        if (!result.has_value()) {
            std::cerr << "MongoDB did not acknowledge the application-user update.\n";
            return 1;
        }

        std::cout << "Provisioned application user '" << arguments->username << "'.\n";
        return 0;
    } catch (const std::exception& error) {
        sodium_memzero(password.data(), password.size());
        std::cerr << "Application-user provisioning failed: " << error.what() << '\n';
        return 1;
    }
}
