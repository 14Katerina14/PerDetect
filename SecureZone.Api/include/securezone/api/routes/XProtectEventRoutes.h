#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"

namespace securezone::api {

class XProtectEventRoutes {
public:
    HttpResponse handleLineCrossing(const HttpRequest& request) const;
};

}
