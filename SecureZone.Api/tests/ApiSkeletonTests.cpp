#include "securezone/api/ApiApplication.h"
#include "securezone/api/http/Router.h"

#include <cassert>
#include <utility>
#include <vector>
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

void qrRouteRequiresConfiguredService() {
    const ApiServer server{};

    const auto response = server.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
        {}
    });

    assert(response.statusCode == 503);
    assert(response.body.find("service_unavailable") != std::string::npos);
}

void qrRouteCallsConfiguredCheckInHandler() {
    bool called = false;
    ApiServer server{
        {},
        [&called](const securezone::qr::QrCheckInCommand& command) {
            called = true;
            assert(command.employeeId == "EMP-001");
            assert(command.zoneId == "ZONE-001");
            assert(command.scannedByUserId == "APP-SCANNER-001");
            return securezone::qr::QrCheckInResult{
                true,
                "started",
                "SESSION-001",
                {}
            };
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
        {}
    });

    assert(called);
    assert(response.statusCode == 201);
    assert(response.body.find(R"("accepted":true)") != std::string::npos);
    assert(response.body.find(R"("status":"started")") != std::string::npos);
    assert(response.body.find(R"("sessionId":"SESSION-001")") != std::string::npos);
}

void xprotectRouteAcceptsWiseAiLineCrossingEvent() {
    const ApiServer server{};

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 202);
    assert(response.body.find(R"("accepted":true)") != std::string::npos);
    assert(response.body.find("xprotect_line_crossing") != std::string::npos);
    assert(response.body.find("Hanwha Vision TNO-C4052T") != std::string::npos);
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

    assert(response.statusCode == 503);
    assert(response.body.find("QR check-in service is not configured") != std::string::npos);
}

void applicationRoutesQrCheckInRequestsToConfiguredHandler() {
    ApiApplication app{
        {},
        {},
        [](const securezone::qr::QrCheckInCommand& command) {
            assert(command.employeeId == "EMP-001");
            assert(command.zoneId == "ZONE-001");
            assert(command.scannedByUserId == "APP-SCANNER-001");
            return securezone::qr::QrCheckInResult{
                true,
                "extended",
                "SESSION-001",
                {}
            };
        }
    };

    const auto response = app.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
        {}
    });

    assert(response.statusCode == 200);
    assert(response.body.find(R"("status":"extended")") != std::string::npos);
}

void qrRouteMapsRejectedStatusesToHttpResponses() {
    const std::vector<std::pair<std::string, int>> cases{
        {"invalid_request", 400},
        {"employee_not_found", 404},
        {"zone_not_found", 404},
        {"scanner_not_found", 404},
        {"employee_inactive", 403},
        {"zone_inactive", 403},
        {"scanner_not_allowed", 403}
    };

    for (const auto& testCase : cases) {
        ApiServer server{
            {},
            [&testCase](const securezone::qr::QrCheckInCommand&) {
                return securezone::qr::QrCheckInResult{
                    false,
                    testCase.first,
                    {},
                    "mapped message"
                };
            }
        };

        const auto response = server.handle({
            "POST",
            "/api/qr/check-in",
            R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
            {}
        });

        assert(response.statusCode == testCase.second);
        assert(response.body.find(testCase.first) != std::string::npos);
        assert(response.body.find("mapped message") != std::string::npos);
    }
}

void applicationRoutesXProtectLineCrossingRequests() {
    const ApiApplication app{};

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1"})",
        {}
    });

    assert(response.statusCode == 202);
    assert(response.body.find(R"("status":"accepted")") != std::string::npos);
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

void xprotectLineCrossingRejectsMissingSource() {
    const ApiApplication app{};

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2"})",
        {}
    });

    assert(response.statusCode == 400);
    assert(response.body.find("missing_source_name") != std::string::npos);
}

void xprotectLineCrossingRejectsUnsupportedEvents() {
    const ApiApplication app{};

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Device.Offline","sourceName":"Camera 1"})",
        {}
    });

    assert(response.statusCode == 400);
    assert(response.body.find("unsupported_xprotect_event") != std::string::npos);
}

}

int main() {
    healthRouteReturnsOk();
    qrRouteRequiresConfiguredService();
    qrRouteCallsConfiguredCheckInHandler();
    xprotectRouteAcceptsWiseAiLineCrossingEvent();
    routerReturnsNotFoundForUnknownPath();
    routerReturnsMethodNotAllowedForKnownPathWithWrongMethod();
    settingsCanBePassedToServer();
    applicationRoutesHealthRequests();
    applicationRoutesQrCheckInRequests();
    applicationRoutesQrCheckInRequestsToConfiguredHandler();
    qrRouteMapsRejectedStatusesToHttpResponses();
    applicationRoutesXProtectLineCrossingRequests();
    applicationKeepsSettingsAndStartupSummary();
    xprotectLineCrossingRejectsMissingSource();
    xprotectLineCrossingRejectsUnsupportedEvents();
}
