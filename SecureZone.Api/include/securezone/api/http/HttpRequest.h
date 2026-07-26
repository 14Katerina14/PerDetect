#pragma once

#include <string>
#include <unordered_map>

namespace securezone::api {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::string remoteAddress;
};

}
