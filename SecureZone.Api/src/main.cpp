#include <iostream>

#include "securezone/api/ApiApplication.h"

int main() {
    const auto app = securezone::api::createApiApplicationFromEnvironment();

    const auto response = app.handle({"GET", "/health", {}, {}});
    std::cout << response.body << '\n';
    std::cout << app.startupSummary() << '\n';
    return response.statusCode == 200 ? 0 : 1;
}
