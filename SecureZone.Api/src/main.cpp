#include <iostream>

#include "securezone/api/ApiApplication.h"
#ifdef SECUREZONE_WITH_HTTP_RUNTIME
#include "securezone/api/http/HttpRuntimeServer.h"
#endif

int main() {
    const auto app = securezone::api::createApiApplicationFromEnvironment();

#ifdef SECUREZONE_WITH_HTTP_RUNTIME
    std::cout << app.startupSummary() << '\n';
    return securezone::api::runHttpRuntimeServer(app) ? 0 : 1;
#else
    const auto response = app.handle({"GET", "/health", {}, {}});
    std::cout << response.body << '\n';
    std::cout << app.startupSummary() << '\n';
    return response.statusCode == 200 ? 0 : 1;
#endif
}
