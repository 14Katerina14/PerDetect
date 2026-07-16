#pragma once

#include "securezone/api/auth/EndpointAuthorizer.h"
#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/query/ManagerReadModels.h"

#include <functional>
#include <vector>

namespace securezone::api {

class AlarmRoutes {
public:
    using ListHandler = std::function<std::vector<query::AlarmView>()>;

    AlarmRoutes() = default;
    AlarmRoutes(
        ListHandler activeHandler,
        ListHandler recentHandler,
        EndpointAuthorizer::Handler authorizeHandler
    );

    HttpResponse handleActive(const HttpRequest& request) const;
    HttpResponse handleRecent(const HttpRequest& request) const;

private:
    HttpResponse handleList(const HttpRequest& request, const ListHandler& handler) const;

    ListHandler activeHandler_;
    ListHandler recentHandler_;
    EndpointAuthorizer::Handler authorizeHandler_;
};

}
