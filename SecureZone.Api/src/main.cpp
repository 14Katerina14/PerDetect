#include <iostream>

#include "securezone/api/ApiServer.h"
#include "securezone/api/ApiSettings.h"

int main() {
    const auto settings = securezone::api::loadApiSettingsFromEnvironment();
    const securezone::api::ApiServer server{settings};

    const auto response = server.handle({"GET", "/health", {}, {}});
    std::cout << response.body << '\n';
    std::cout << "SecureZone API skeleton configured for "
              << settings.host << ':' << settings.port << '\n';
    return response.statusCode == 200 ? 0 : 1;
}
