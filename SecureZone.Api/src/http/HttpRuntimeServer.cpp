#include "securezone/api/http/HttpRuntimeServer.h"

#include <httplib.h>

#include <iostream>

namespace securezone::api {
namespace {

HttpRequest toApiRequest(const httplib::Request& request) {
    HttpRequest result{};
    result.method = request.method;
    result.path = request.path;
    result.body = request.body;
    result.remoteAddress = request.remote_addr;

    for (const auto& header : request.headers) {
        result.headers.emplace(header.first, header.second);
    }

    return result;
}

void writeApiResponse(const HttpResponse& response, httplib::Response& target) {
    target.status = response.statusCode;
    target.set_content(response.body, response.contentType);
}

}

bool runHttpRuntimeServer(const ApiApplication& application) {
    httplib::Server server;
    const auto handle = [&application](const httplib::Request& request, httplib::Response& response) {
        writeApiResponse(application.handle(toApiRequest(request)), response);
    };

    server.Get("/health", handle);
    server.Post("/api/auth/login", handle);
    server.Post("/api/qr/check-in", handle);
    server.Post("/api/xprotect/line-crossing", handle);
    server.Post("/api/xprotect/object-observations", handle);

    const auto& settings = application.settings();
    if (!server.bind_to_port(settings.host, settings.port)) {
        std::cerr << "SecureZone API failed to bind to "
                  << settings.host << ':' << settings.port << std::endl;
        return false;
    }

    std::cout << "SecureZone API listening on http://"
              << settings.host << ':' << settings.port << std::endl;

    return server.listen_after_bind();
}

}
