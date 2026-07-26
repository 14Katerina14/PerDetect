#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace securezone::domain {

enum class AppUserRole { Worker, Scanner, Manager, Admin };
enum class AppUserStatus { Active, Inactive };

inline std::string_view toString(AppUserRole role) {
    switch (role) {
        case AppUserRole::Worker: return "worker";
        case AppUserRole::Scanner: return "scanner";
        case AppUserRole::Manager: return "manager";
        case AppUserRole::Admin: return "admin";
    }

    return "";
}

inline std::optional<AppUserRole> appUserRoleFromString(std::string_view value) {
    if (value == "worker") return AppUserRole::Worker;
    if (value == "scanner") return AppUserRole::Scanner;
    if (value == "manager") return AppUserRole::Manager;
    if (value == "admin") return AppUserRole::Admin;
    return std::nullopt;
}

struct AppUser {
    std::string userId;
    std::string username;
    std::string employeeId;
    std::string passwordHash;
    AppUserRole role{AppUserRole::Scanner};
    AppUserStatus status{AppUserStatus::Active};

    bool canScanQr() const {
        return status == AppUserStatus::Active && role == AppUserRole::Scanner;
    }
};

}
