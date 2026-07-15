#pragma once

#include <string>

namespace securezone::api {

struct HttpResponse {
    int statusCode{200};
    std::string contentType{"application/json"};
    std::string body;
};

}
