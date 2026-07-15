#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"

namespace securezone::api {

class QrRoutes {
public:
    HttpResponse handleCheckIn(const HttpRequest& request) const;
};

}
