#include "securezone/api/http/Router.h"

#include <utility>

#include "securezone/api/ApiResponse.h"

namespace securezone::api {

void Router::get(std::string path, Handler handler) {
    routes_[{"GET", std::move(path)}] = std::move(handler);
}

void Router::post(std::string path, Handler handler) {
    routes_[{"POST", std::move(path)}] = std::move(handler);
}

HttpResponse Router::route(const HttpRequest& request) const {
    const auto route = routes_.find({request.method, request.path});
    if (route != routes_.end()) {
        return route->second(request);
    }

    for (const auto& [key, handler] : routes_) {
        const auto& [method, path] = key;
        if (path == request.path && method != request.method) {
            return jsonMethodNotAllowed(R"({"error":"method_not_allowed"})");
        }
    }

    return jsonNotFound(R"({"error":"not_found"})");
}

}
