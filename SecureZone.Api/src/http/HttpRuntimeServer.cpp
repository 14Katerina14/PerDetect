#include "securezone/api/http/HttpRuntimeServer.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

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

std::string utcTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream value;
    value << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return value.str();
}

std::string bodyIdFrom(const httplib::Request& request, const char* field) {
    if (request.body.empty()) return "-";
    const auto payload = nlohmann::json::parse(request.body, nullptr, false);
    if (payload.is_object()) {
        const auto value = payload.find(field);
        if (value != payload.end() && value->is_string() && !value->get_ref<const std::string&>().empty()) {
            return value->get<std::string>();
        }
    }
    return "-";
}

void logRequest(
    const httplib::Request& request,
    int status,
    std::chrono::steady_clock::time_point startedAt
) {
    static std::mutex logMutex;
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startedAt
    ).count();

    std::ostringstream line;
    line << utcTimestamp()
         << " request method=" << request.method
         << " path=" << request.path
         << " status=" << status
         << " durationMs=" << duration
         << " remote=" << (request.remote_addr.empty() ? "-" : request.remote_addr)
         << " eventId=" << bodyIdFrom(request, "eventId")
         << " requestId=" << bodyIdFrom(request, "requestId");

    std::lock_guard<std::mutex> lock{logMutex};
    std::clog << line.str() << std::endl;
}

bool isRegisteredRuntimeRoute(const httplib::Request& request) {
    return (request.method == "GET" && (
            request.path == "/health"
            || request.path == "/version"
            || request.path == "/api/alarms/active"
            || request.path == "/api/alarms/recent"
        ))
        || (request.method == "POST" && (
            request.path == "/api/auth/login"
            || request.path == "/api/qr/check-in"
            || request.path == "/api/xprotect/line-crossing"
            || request.path == "/api/xprotect/object-observations"
        ));
}

}

bool runHttpRuntimeServer(const ApiApplication& application) {
    httplib::Server server;
    const auto handle = [&application](const httplib::Request& request, httplib::Response& response) {
        const auto startedAt = std::chrono::steady_clock::now();
        try {
            writeApiResponse(application.handle(toApiRequest(request)), response);
        } catch (const std::exception&) {
            response.status = 500;
            response.set_content(R"({"error":"internal_server_error"})", "application/json");
            std::cerr << utcTimestamp() << " request_error method=" << request.method
                      << " path=" << request.path << " message=unhandled_exception" << std::endl;
        }
        logRequest(request, response.status, startedAt);
    };

    server.Get("/health", handle);
    server.Get("/version", handle);
    server.Get("/api/alarms/active", handle);
    server.Get("/api/alarms/recent", handle);
    server.Post("/api/auth/login", handle);
    server.Post("/api/qr/check-in", handle);
    server.Post("/api/xprotect/line-crossing", handle);
    server.Post("/api/xprotect/object-observations", handle);
    server.set_error_handler([&application](const httplib::Request& request, httplib::Response& response) {
        if (isRegisteredRuntimeRoute(request)) {
            return;
        }

        const auto startedAt = std::chrono::steady_clock::now();
        if (response.status == 404) {
            writeApiResponse(application.handle(toApiRequest(request)), response);
        }
        logRequest(request, response.status, startedAt);
    });

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
