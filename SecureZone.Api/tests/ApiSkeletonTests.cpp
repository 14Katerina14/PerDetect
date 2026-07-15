#include "securezone/api/ApiApplication.h"
#include "securezone/api/http/Router.h"

#include <cassert>
#include <string>

namespace {

using namespace securezone::api;

void healthRouteReturnsOk() {
    const ApiServer server{};

    const auto response = server.handle({"GET", "/health", {}, {}});

    assert(response.statusCode == 200);
    assert(response.contentType == "application/json");
    assert(response.body == R"({"status":"ok","service":"securezone-api"})");
}

void qrRouteIsReservedForQrImplementation() {
    const ApiServer server{};

    const auto response = server.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
        {}
    });

    assert(response.statusCode == 501);
    assert(response.body.find("not_implemented") != std::string::npos);
}

void xprotectRouteIsReservedForEventImplementation() {
    const ApiServer server{};

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2"})",
        {}
    });

    assert(response.statusCode == 501);
    assert(response.body.find("not_implemented") != std::string::npos);
}

void routerReturnsNotFoundForUnknownPath() {
    const ApiServer server{};

    const auto response = server.handle({"GET", "/missing", {}, {}});

    assert(response.statusCode == 404);
    assert(response.body == R"({"error":"not_found"})");
}

void routerReturnsMethodNotAllowedForKnownPathWithWrongMethod() {
    const ApiServer server{};

    const auto response = server.handle({"POST", "/health", {}, {}});

    assert(response.statusCode == 405);
    assert(response.body == R"({"error":"method_not_allowed"})");
}

void settingsCanBePassedToServer() {
    ApiSettings settings{};
    settings.host = "127.0.0.1";
    settings.port = 9090;
    settings.mongoDatabaseName = "securezone_test";

    const ApiServer server{settings};

    assert(server.settings().host == "127.0.0.1");
    assert(server.settings().port == 9090);
    assert(server.settings().mongoDatabaseName == "securezone_test");
}

void applicationRoutesHealthRequests() {
    const ApiApplication app{};

    const auto response = app.handle({"GET", "/health", {}, {}});

    assert(response.statusCode == 200);
    assert(response.body.find("securezone-api") != std::string::npos);
}

void applicationRoutesQrCheckInRequests() {
    const ApiApplication app{};

    const auto response = app.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","qrToken":"token"})",
        {}
    });

    assert(response.statusCode == 501);
    assert(response.body.find("QR check-in endpoint") != std::string::npos);
}

void applicationRoutesXProtectLineCrossingRequests() {
    const ApiApplication app{};

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2"})",
        {}
    });

    assert(response.statusCode == 501);
    assert(response.body.find("XProtect LineCrossing endpoint") != std::string::npos);
}

void applicationKeepsSettingsAndStartupSummary() {
    ApiSettings settings{};
    settings.host = "127.0.0.1";
    settings.port = 9091;
    settings.mongoDatabaseName = "securezone_local";
    ApiApplicationInfo info{};
    info.version = "test";

    const ApiApplication app{settings, info};

    assert(app.settings().host == "127.0.0.1");
    assert(app.info().version == "test");
    assert(app.startupSummary().find("127.0.0.1:9091") != std::string::npos);
    assert(app.startupSummary().find("securezone_local") != std::string::npos);
}

}

int main() {
    healthRouteReturnsOk();
    qrRouteIsReservedForQrImplementation();
    xprotectRouteIsReservedForEventImplementation();
    routerReturnsNotFoundForUnknownPath();
    routerReturnsMethodNotAllowedForKnownPathWithWrongMethod();
    settingsCanBePassedToServer();
    applicationRoutesHealthRequests();
    applicationRoutesQrCheckInRequests();
    applicationRoutesXProtectLineCrossingRequests();
    applicationKeepsSettingsAndStartupSummary();
}
