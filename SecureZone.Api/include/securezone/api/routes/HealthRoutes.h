#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"

namespace securezone::api {

class HealthRoutes {
public:
    HttpResponse handleHealth(const HttpRequest& request) const;
};

}
