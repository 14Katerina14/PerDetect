#pragma once

#include <string>

namespace securezone::domain {

enum class AppUserRole { Scanner, Manager, Admin };
enum class AppUserStatus { Active, Inactive };

struct AppUser {
    std::string userId;
    std::string username;
    AppUserRole role{AppUserRole::Scanner};
    AppUserStatus status{AppUserStatus::Active};

    bool canScanQr() const {
        return status == AppUserStatus::Active && role == AppUserRole::Scanner;
    }
};

}
