#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/api/events/XProtectLineCrossingEvent.h"

#include <functional>

namespace securezone::api {

class XProtectEventRoutes {
public:
    using LineCrossingHandler = std::function<XProtectLineCrossingResult(const XProtectLineCrossingEvent&)>;

    XProtectEventRoutes() = default;
    explicit XProtectEventRoutes(LineCrossingHandler lineCrossingHandler);

    HttpResponse handleLineCrossing(const HttpRequest& request) const;

private:
    LineCrossingHandler lineCrossingHandler_;
};

}
