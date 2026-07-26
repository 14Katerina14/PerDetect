#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/api/events/XProtectLineCrossingEvent.h"

#include <functional>
#include <string>

namespace securezone::api {

class XProtectEventRoutes {
public:
    using LineCrossingHandler = std::function<XProtectLineCrossingResult(const XProtectLineCrossingEvent&)>;

    XProtectEventRoutes() = default;
    explicit XProtectEventRoutes(
        LineCrossingHandler lineCrossingHandler,
        std::string apiKey = {}
    );

    HttpResponse handleLineCrossing(const HttpRequest& request) const;

private:
    LineCrossingHandler lineCrossingHandler_;
    std::string apiKey_;
};

}
