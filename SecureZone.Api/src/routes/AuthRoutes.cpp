#include "securezone/api/routes/AuthRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/domain/AppUser.h"

#include <nlohmann/json.hpp>

#include <utility>

namespace securezone::api {

AuthRoutes::AuthRoutes(LoginHandler loginHandler)
    : loginHandler_{std::move(loginHandler)} {
}

HttpResponse AuthRoutes::handleLogin(const HttpRequest& request) const {
    const auto body = nlohmann::json::parse(request.body, nullptr, false);
    if (body.is_discarded()
        || !body.is_object()
        || !body.contains("username")
        || !body["username"].is_string()
        || !body.contains("password")
        || !body["password"].is_string()) {
        return jsonResponse(400, R"({"error":"invalid_request"})");
    }

    auth::LoginCommand command{
        body["username"].get<std::string>(),
        body["password"].get<std::string>()
    };
    if (command.username.empty() || command.password.empty()) {
        return jsonResponse(400, R"({"error":"invalid_request"})");
    }

    if (!loginHandler_) {
        return jsonResponse(503, R"({"error":"authentication_unavailable"})");
    }

    const auto result = loginHandler_(command);
    if (result.status == auth::LoginStatus::InvalidCredentials) {
        return jsonResponse(401, R"({"error":"invalid_credentials"})");
    }
    if (!result.succeeded()) {
        return jsonResponse(503, R"({"error":"authentication_unavailable"})");
    }

    const auto& principal = *result.principal;
    nlohmann::json response{
        {"accessToken", result.accessToken.value},
        {"tokenType", "Bearer"},
        {"expiresIn", result.accessToken.expiresIn.count()},
        {"user", {
            {"userId", principal.userId},
            {"username", principal.username},
            {"role", std::string{domain::toString(principal.role)}},
            {"employeeId", principal.employeeId}
        }}
    };
    return jsonResponse(200, response.dump());
}

}
