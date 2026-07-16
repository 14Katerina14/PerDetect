#pragma once

#include "securezone/auth/AuthenticationTypes.h"

namespace securezone::auth {

enum class AuthorizationPolicy {
    Worker,
    Scanner,
    ManagerOrAdmin,
    AdminOnly
};

inline bool isAuthorized(
    const AuthenticatedPrincipal& principal,
    AuthorizationPolicy policy
) {
    using domain::AppUserRole;

    switch (policy) {
        case AuthorizationPolicy::Worker:
            return principal.role == AppUserRole::Worker && !principal.employeeId.empty();
        case AuthorizationPolicy::Scanner:
            return principal.role == AppUserRole::Scanner;
        case AuthorizationPolicy::ManagerOrAdmin:
            return principal.role == AppUserRole::Manager
                || principal.role == AppUserRole::Admin;
        case AuthorizationPolicy::AdminOnly:
            return principal.role == AppUserRole::Admin;
    }

    return false;
}

}
