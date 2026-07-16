#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/auth/AuthenticationTypes.h"

#include <functional>

namespace securezone::api {

class AuthRoutes {
public:
    using LoginHandler = std::function<auth::LoginResult(const auth::LoginCommand&)>;

    AuthRoutes() = default;
    explicit AuthRoutes(LoginHandler loginHandler);

    HttpResponse handleLogin(const HttpRequest& request) const;

private:
    LoginHandler loginHandler_;
};

}
