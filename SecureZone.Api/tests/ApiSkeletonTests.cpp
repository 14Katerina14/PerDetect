#include "securezone/api/ApiApplication.h"
#include "securezone/api/handlers/XProtectLineCrossingHandler.h"
#include "securezone/api/http/Router.h"
#include "securezone/api/runtime/ApiRuntimeComposition.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/Zone.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>
#include <string>

namespace {

using namespace securezone::api;
namespace domain = securezone::domain;
namespace xprotect = securezone::xprotect;
using Clock = std::chrono::system_clock;

const auto XProtectEventTime = Clock::time_point{} + std::chrono::minutes{10};
const std::string WiseAiLineCrossingEvent = "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2";
const std::string CameraSource = "Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1";

domain::Zone mappedXProtectZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Machine A dangerous zone";
    zone.cameraId = "CAM-001";
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.xprotectEventName = WiseAiLineCrossingEvent;
    return zone;
}

domain::PresenceSession activePresenceSession() {
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.sourceCheckinId = "CHECKIN-001";
    session.startedAt = XProtectEventTime - std::chrono::minutes{5};
    session.expiresAt = XProtectEventTime + std::chrono::minutes{5};
    session.status = domain::PresenceSessionStatus::Active;
    return session;
}

std::optional<Clock::time_point> fixedXProtectEventTime(const std::string&) {
    return XProtectEventTime;
}

domain::Employee activeEmployee() {
    domain::Employee employee{};
    employee.employeeId = "EMP-001";
    employee.fullName = "Ivan Petrov";
    employee.roles = {"maintenance"};
    employee.status = domain::EmployeeStatus::Active;
    return employee;
}

domain::AppUser scannerUser() {
    domain::AppUser appUser{};
    appUser.userId = "APP-SCANNER-001";
    appUser.username = "scanner";
    appUser.role = domain::AppUserRole::Scanner;
    appUser.status = domain::AppUserStatus::Active;
    return appUser;
}

Clock::time_point compositionEventTime() {
    return *XProtectLineCrossingHandler::parseReceivedAt("2026-07-15T10:30:00Z");
}

void xprotectTimestampParserAcceptsDotNetRoundTripUtc() {
    const auto wholeSecond = XProtectLineCrossingHandler::parseReceivedAt(
        "2026-07-15T10:30:00Z"
    );
    const auto dotNetRoundTrip = XProtectLineCrossingHandler::parseReceivedAt(
        "2026-07-15T10:30:00.1234567Z"
    );

    assert(wholeSecond.has_value());
    assert(dotNetRoundTrip.has_value());
    assert(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            *dotNetRoundTrip - *wholeSecond
        ).count() == 123456700
    );
}

domain::PresenceSession activeCompositionPresenceSession() {
    auto session = activePresenceSession();
    session.startedAt = compositionEventTime() - std::chrono::minutes{5};
    session.expiresAt = compositionEventTime() + std::chrono::minutes{5};
    return session;
}

ApiRuntimeConfig compositionConfig(bool includePresenceSession) {
    ApiRuntimeConfig config{};
    config.employees = {activeEmployee()};
    config.appUsers = {scannerUser()};
    config.zones = {mappedXProtectZone()};
    config.xprotectZoneMappings = {{
        WiseAiLineCrossingEvent,
        CameraSource,
        "ZONE-001"
    }};

    if (includePresenceSession) {
        config.presenceSessions = {activeCompositionPresenceSession()};
    }

    return config;
}

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

void xprotectRouteRequiresConfiguredHandler() {
    const ApiServer server{};

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 503);
    assert(response.body.find("service_unavailable") != std::string::npos);
}

void xprotectRouteCallsConfiguredHandler() {
    bool called = false;
    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            [&called](const XProtectLineCrossingEvent& event) {
                called = true;
                assert(event.eventId == "EVENT-001");
                assert(event.eventName == "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2");
                assert(event.sourceName == "Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1");
                assert(event.receivedAt == "2026-07-15T10:30:00Z");
                return XProtectLineCrossingResult{
                    true,
                    "processed",
                    "violation",
                    "ZONE-001",
                    {},
                    {},
                    "No active presence session for zone."
                };
            }
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventId":"EVENT-001","eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(called);
    assert(response.statusCode == 200);
    assert(response.body.find(R"("accepted":true)") != std::string::npos);
    assert(response.body.find(R"("decision":"violation")") != std::string::npos);
    assert(response.body.find("No active presence session") != std::string::npos);
    assert(response.body.find(R"("eventId":"EVENT-001")") != std::string::npos);
    assert(response.body.find("xprotect_line_crossing") != std::string::npos);
}

void xprotectRouteRequiresConfiguredApiKey() {
    ApiSettings settings{};
    settings.xprotectApiKey = "test-only-api-key";
    bool called = false;
    ApiServer server{
        settings,
        ApiRouteHandlers{
            {},
            [&called](const XProtectLineCrossingEvent&) {
                called = true;
                return XProtectLineCrossingResult{
                    true,
                    "processed",
                    "violation",
                    "ZONE-001",
                    {},
                    {},
                    "No active QR presence session found for zone."
                };
            }
        }
    };

    const std::string body =
        R"({"eventId":"EVENT-001","eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Camera 1","receivedAt":"2026-07-15T10:30:00Z"})";

    const auto unauthorized = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        body,
        {}
    });

    assert(unauthorized.statusCode == 401);
    assert(!called);

    const auto authorized = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        body,
        {{"X-SecureZone-Api-Key", "test-only-api-key"}}
    });

    assert(authorized.statusCode == 200);
    assert(called);
    assert(authorized.body.find(R"("decision":"violation")") != std::string::npos);
}

void xprotectRouteDoesNotEvaluateDuplicateEventTwice() {
    int evaluations = 0;
    xprotect::XProtectLineCrossingService service{
        [&evaluations](const xprotect::XProtectLineCrossingCommand&) {
            ++evaluations;
            return std::optional<domain::Zone>{mappedXProtectZone()};
        },
        [](const domain::Zone&, Clock::time_point) {
            return std::optional<domain::PresenceSession>{};
        }
    };
    XProtectLineCrossingHandler handler{service, fixedXProtectEventTime};
    ApiServer server{{}, ApiRouteHandlers{{}, handler}};

    const HttpRequest request{
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventId":"EVENT-DUPLICATE","eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    };

    const auto first = server.handle(request);
    const auto duplicate = server.handle(request);

    assert(evaluations == 1);
    assert(first.statusCode == 200);
    assert(first.body.find(R"("duplicate":false)") != std::string::npos);
    assert(duplicate.statusCode == 200);
    assert(duplicate.body.find(R"("duplicate":true)") != std::string::npos);
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

    assert(response.statusCode == 503);
    assert(response.body.find("XProtect line crossing handler is not configured") != std::string::npos);
}

void applicationRoutesXProtectLineCrossingRequestsToConfiguredHandler() {
    ApiApplication app{
        {},
        {},
        ApiRouteHandlers{
            {},
            [](const XProtectLineCrossingEvent& event) {
                assert(event.sourceName == "Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1");
                return XProtectLineCrossingResult{
                    true,
                    "queued",
                    "pending",
                    {},
                    {},
                    {},
                    "Decision flow queued."
                };
            }
        }
    };

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1"})",
        {}
    });

    assert(response.statusCode == 202);
    assert(response.body.find(R"("status":"queued")") != std::string::npos);
    assert(response.body.find(R"("decision":"pending")") != std::string::npos);
}

void xprotectRouteMapsHandlerResultsToHttpResponses() {
    const std::vector<std::pair<std::string, int>> cases{
        {"invalid_request", 400},
        {"zone_not_found", 404},
        {"handler_error", 500},
        {"rejected", 400}
    };

    for (const auto& testCase : cases) {
        ApiServer server{
            {},
            ApiRouteHandlers{
                {},
                [&testCase](const XProtectLineCrossingEvent&) {
                    return XProtectLineCrossingResult{
                        false,
                        testCase.first,
                        "none",
                        {},
                        {},
                        {},
                        "mapped xprotect message"
                    };
                }
            }
        };

        const auto response = server.handle({
            "POST",
            "/api/xprotect/line-crossing",
            R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1"})",
            {}
        });

        assert(response.statusCode == testCase.second);
        assert(response.body.find(testCase.first) != std::string::npos);
        assert(response.body.find("mapped xprotect message") != std::string::npos);
    }
}

void xprotectLineCrossingCreatesViolationWhenPresenceIsMissing() {
    xprotect::XProtectLineCrossingService service{
        [](const xprotect::XProtectLineCrossingCommand&) {
            return std::optional<domain::Zone>{mappedXProtectZone()};
        },
        [](const domain::Zone&, Clock::time_point) {
            return std::optional<domain::PresenceSession>{};
        }
    };

    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            XProtectLineCrossingHandler{service, fixedXProtectEventTime}
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 200);
    assert(response.body.find(R"("accepted":true)") != std::string::npos);
    assert(response.body.find(R"("decision":"violation")") != std::string::npos);
    assert(response.body.find(R"("zoneId":"ZONE-001")") != std::string::npos);
    assert(response.body.find("No active QR presence session") != std::string::npos);
}

void xprotectLineCrossingAllowsActivePresence() {
    xprotect::XProtectLineCrossingService service{
        [](const xprotect::XProtectLineCrossingCommand&) {
            return std::optional<domain::Zone>{mappedXProtectZone()};
        },
        [](const domain::Zone&, Clock::time_point) {
            return std::optional<domain::PresenceSession>{activePresenceSession()};
        }
    };

    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            XProtectLineCrossingHandler{service, fixedXProtectEventTime}
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 200);
    assert(response.body.find(R"("decision":"allowed")") != std::string::npos);
    assert(response.body.find(R"("zoneId":"ZONE-001")") != std::string::npos);
    assert(response.body.find(R"("sessionId":"SESSION-001")") != std::string::npos);
    assert(response.body.find(R"("employeeId":"EMP-001")") != std::string::npos);
}

void xprotectLineCrossingReturnsNotFoundForUnmappedZone() {
    xprotect::XProtectLineCrossingService service{
        [](const xprotect::XProtectLineCrossingCommand&) {
            return std::optional<domain::Zone>{};
        },
        [](const domain::Zone&, Clock::time_point) {
            return std::optional<domain::PresenceSession>{};
        }
    };

    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            XProtectLineCrossingHandler{service, fixedXProtectEventTime}
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 404);
    assert(response.body.find(R"("status":"zone_not_found")") != std::string::npos);
}

void xprotectLineCrossingReturnsServerErrorForHandlerError() {
    xprotect::XProtectLineCrossingService service{
        xprotect::XProtectLineCrossingService::ZoneResolver{},
        xprotect::XProtectLineCrossingService::ActivePresenceResolver{}
    };

    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            XProtectLineCrossingHandler{service, fixedXProtectEventTime}
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 500);
    assert(response.body.find(R"("status":"handler_error")") != std::string::npos);
}

void runtimeCompositionWiresQrCheckInHandler() {
    const ApiRuntimeComposition composition{compositionConfig(false)};
    const auto app = composition.createApplication();

    const auto response = app.handle({
        "POST",
        "/api/qr/check-in",
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"})",
        {}
    });

    assert(response.statusCode == 201);
    assert(response.body.find(R"("accepted":true)") != std::string::npos);
    assert(response.body.find(R"("status":"started")") != std::string::npos);
}

void runtimeCompositionMapsXProtectEventToViolationWithoutPresence() {
    const ApiRuntimeComposition composition{compositionConfig(false)};
    const auto app = composition.createApplication();

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 200);
    assert(response.body.find(R"("decision":"violation")") != std::string::npos);
    assert(response.body.find(R"("zoneId":"ZONE-001")") != std::string::npos);
}

void runtimeCompositionMapsXProtectEventToAllowedWithActivePresence() {
    const ApiRuntimeComposition composition{compositionConfig(true)};
    const auto app = composition.createApplication();

    const auto response = app.handle({
        "POST",
        "/api/xprotect/line-crossing",
        R"({"eventName":"Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2","sourceName":"Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1","receivedAt":"2026-07-15T10:30:00Z"})",
        {}
    });

    assert(response.statusCode == 200);
    assert(response.body.find(R"("decision":"allowed")") != std::string::npos);
    assert(response.body.find(R"("sessionId":"SESSION-001")") != std::string::npos);
    assert(response.body.find(R"("employeeId":"EMP-001")") != std::string::npos);
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

void cameraObjectObservationRouteForwardsIdentityEvidence() {
    bool called = false;
    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            {},
            [&called](const securezone::identity::CameraObjectObservation& observation) {
                called = true;
                assert(observation.cameraId == "CAM-001");
                assert(observation.objectId == "OBJECT-42");
                assert(observation.objectType == "Human");
                return securezone::identity::UnidentifiedPersonWatchdogResult{
                    true,
                    "processed",
                    "pending_identity",
                    "SAFE-001",
                    "The QR identity grace period is still active.",
                    "IDENTITY-CAM-001-OBJECT-42",
                    false
                };
            }
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/object-observations",
        R"({"cameraId":"CAM-001","objectId":"OBJECT-42","objectType":"Human","observedAt":"2026-07-16T00:30:00Z"})",
        {}
    });

    assert(called);
    assert(response.statusCode == 200);
    assert(response.body.find("pending_identity") != std::string::npos);
    assert(response.body.find("SAFE-001") != std::string::npos);
}

void cameraObjectObservationRouteRejectsIncompleteEvidence() {
    ApiServer server{
        {},
        ApiRouteHandlers{{}, {}, [](const securezone::identity::CameraObjectObservation&) {
            return securezone::identity::UnidentifiedPersonWatchdogResult{};
        }}
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/object-observations",
        R"({"cameraId":"CAM-001","objectType":"Human","observedAt":"2026-07-16T00:30:00Z"})",
        {}
    });

    assert(response.statusCode == 400);
    assert(response.body.find("invalid_request") != std::string::npos);
}

void cameraObjectObservationRouteForwardsLostStatus() {
    bool lost = false;
    ApiServer server{
        {},
        ApiRouteHandlers{
            {},
            {},
            [&lost](const securezone::identity::CameraObjectObservation& observation) {
                lost = observation.status
                    == securezone::identity::CameraObjectObservationStatus::Lost;
                return securezone::identity::UnidentifiedPersonWatchdogResult{
                    true, "processed", "cleared", "SAFE-001", "Person left.",
                    "IDENTITY-CAM-001-OBJECT-42", false
                };
            }
        }
    };

    const auto response = server.handle({
        "POST",
        "/api/xprotect/object-observations",
        R"({"cameraId":"CAM-001","objectId":"OBJECT-42","objectType":"Human","observedAt":"2026-07-16T00:32:10Z","status":"lost"})",
        {}
    });

    assert(lost);
    assert(response.statusCode == 200);
    assert(response.body.find(R"("decision":"cleared")") != std::string::npos);
}

}

int main() {
    xprotectTimestampParserAcceptsDotNetRoundTripUtc();
    healthRouteReturnsOk();
    qrRouteRequiresConfiguredService();
    qrRouteCallsConfiguredCheckInHandler();
    xprotectRouteRequiresConfiguredHandler();
    xprotectRouteCallsConfiguredHandler();
    xprotectRouteRequiresConfiguredApiKey();
    xprotectRouteDoesNotEvaluateDuplicateEventTwice();
    routerReturnsNotFoundForUnknownPath();
    routerReturnsMethodNotAllowedForKnownPathWithWrongMethod();
    settingsCanBePassedToServer();
    applicationRoutesHealthRequests();
    applicationRoutesQrCheckInRequests();
    applicationRoutesQrCheckInRequestsToConfiguredHandler();
    qrRouteMapsRejectedStatusesToHttpResponses();
    applicationRoutesXProtectLineCrossingRequests();
    applicationRoutesXProtectLineCrossingRequestsToConfiguredHandler();
    xprotectRouteMapsHandlerResultsToHttpResponses();
    applicationKeepsSettingsAndStartupSummary();
    xprotectLineCrossingRejectsMissingSource();
    xprotectLineCrossingRejectsUnsupportedEvents();
    xprotectLineCrossingCreatesViolationWhenPresenceIsMissing();
    xprotectLineCrossingAllowsActivePresence();
    xprotectLineCrossingReturnsNotFoundForUnmappedZone();
    xprotectLineCrossingReturnsServerErrorForHandlerError();
    runtimeCompositionWiresQrCheckInHandler();
    runtimeCompositionMapsXProtectEventToViolationWithoutPresence();
    runtimeCompositionMapsXProtectEventToAllowedWithActivePresence();
    cameraObjectObservationRouteForwardsIdentityEvidence();
    cameraObjectObservationRouteRejectsIncompleteEvidence();
    cameraObjectObservationRouteForwardsLostStatus();
}
