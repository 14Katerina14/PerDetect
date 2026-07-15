#pragma once

#include <functional>
#include <map>
#include <string>

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"

namespace securezone::api {

class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    void get(std::string path, Handler handler);
    void post(std::string path, Handler handler);

    HttpResponse route(const HttpRequest& request) const;

private:
    using RouteKey = std::pair<std::string, std::string>;
    std::map<RouteKey, Handler> routes_;
};

}
