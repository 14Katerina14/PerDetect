#include "securezone/api/runtime/ApiRuntimeComposition.h"

#include "securezone/api/handlers/XProtectLineCrossingHandler.h"
#include "securezone/api/auth/EndpointAuthorizer.h"
#include "securezone/auth/AuthenticationService.h"
#include "securezone/domain/QrCheckIn.h"
#include "securezone/presence/PresenceSessionService.h"
#include "securezone/qr/QrCheckInService.h"
#include "securezone/repository/IAppUserRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IPresenceSessionRepository.h"
#include "securezone/repository/IQrCheckinRepository.h"
#include "securezone/repository/IZoneRepository.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace securezone::api {
namespace {

class InMemoryEmployeeRepository final : public repository::IEmployeeRepository {
public:
    explicit InMemoryEmployeeRepository(std::vector<domain::Employee> employees)
        : employees_{std::move(employees)} {
    }

    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override {
        const auto match = std::find_if(
            employees_.begin(),
            employees_.end(),
            [&employeeId](const domain::Employee& employee) {
                return employee.employeeId == employeeId;
            }
        );

        if (match == employees_.end()) {
            return std::nullopt;
        }

        return *match;
    }

private:
    std::vector<domain::Employee> employees_;
};

class InMemoryAppUserRepository final : public repository::IAppUserRepository {
public:
    explicit InMemoryAppUserRepository(std::vector<domain::AppUser> appUsers)
        : appUsers_{std::move(appUsers)} {
    }

    std::optional<domain::AppUser> findByUserId(
        const std::string& userId
    ) const override {
        const auto match = std::find_if(
            appUsers_.begin(),
            appUsers_.end(),
            [&userId](const domain::AppUser& appUser) {
                return appUser.userId == userId;
            }
        );

        if (match == appUsers_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    std::optional<domain::AppUser> findByUsername(
        const std::string& username
    ) const override {
        const auto match = std::find_if(
            appUsers_.begin(),
            appUsers_.end(),
            [&username](const domain::AppUser& appUser) {
                return appUser.username == username;
            }
        );

        if (match == appUsers_.end()) {
            return std::nullopt;
        }

        return *match;
    }

private:
    std::vector<domain::AppUser> appUsers_;
};

class InMemoryZoneRepository final : public repository::IZoneRepository {
public:
    explicit InMemoryZoneRepository(std::vector<domain::Zone> zones)
        : zones_{std::move(zones)} {
    }

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        const auto match = find(zoneId);
        if (match == zones_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        const auto zone = findByZoneId(zoneId);
        if (!zone.has_value() || zone->status != domain::ZoneStatus::Active) {
            return std::nullopt;
        }

        return zone;
    }

    std::optional<domain::Zone> findActiveByXProtectEventName(
        const std::string& xprotectEventName
    ) const override {
        const auto match = std::find_if(
            zones_.begin(),
            zones_.end(),
            [&xprotectEventName](const domain::Zone& zone) {
                return zone.xprotectEventName == xprotectEventName
                    && zone.status == domain::ZoneStatus::Active;
            }
        );

        if (match == zones_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    bool save(const domain::Zone& zone) override {
        const auto match = find(zone.zoneId);
        if (match == zones_.end()) {
            zones_.push_back(zone);
            return true;
        }

        *match = zone;
        return true;
    }

private:
    std::vector<domain::Zone>::iterator find(const std::string& zoneId) {
        return std::find_if(
            zones_.begin(),
            zones_.end(),
            [&zoneId](const domain::Zone& zone) {
                return zone.zoneId == zoneId;
            }
        );
    }

    std::vector<domain::Zone>::const_iterator find(const std::string& zoneId) const {
        return std::find_if(
            zones_.begin(),
            zones_.end(),
            [&zoneId](const domain::Zone& zone) {
                return zone.zoneId == zoneId;
            }
        );
    }

    std::vector<domain::Zone> zones_;
};

class InMemoryQrCheckinRepository final : public repository::IQrCheckinRepository {
public:
    void create(const domain::QrCheckin& qrCheckin) override {
        qrCheckins_.push_back(qrCheckin);
    }

    std::optional<domain::QrCheckin> findByCheckinId(
        const std::string& checkinId
    ) const override {
        const auto match = std::find_if(
            qrCheckins_.begin(),
            qrCheckins_.end(),
            [&checkinId](const domain::QrCheckin& qrCheckin) {
                return qrCheckin.checkInId == checkinId;
            }
        );

        if (match == qrCheckins_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    std::optional<domain::QrCheckin> findLatestActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override {
        for (auto qrCheckin = qrCheckins_.rbegin(); qrCheckin != qrCheckins_.rend(); ++qrCheckin) {
            if (qrCheckin->employeeId == employeeId
                && qrCheckin->zoneId == zoneId
                && qrCheckin->status == domain::QrCheckInStatus::Active) {
                return *qrCheckin;
            }
        }

        return std::nullopt;
    }

private:
    std::vector<domain::QrCheckin> qrCheckins_;
};

class InMemoryPresenceSessionRepository final : public repository::IPresenceSessionRepository {
public:
    explicit InMemoryPresenceSessionRepository(std::vector<domain::PresenceSession> sessions)
        : sessions_{std::move(sessions)} {
    }

    std::optional<domain::PresenceSession> findActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override {
        const auto match = std::find_if(
            sessions_.begin(),
            sessions_.end(),
            [&employeeId, &zoneId](const domain::PresenceSession& session) {
                return session.employeeId == employeeId
                    && session.zoneId == zoneId
                    && session.status == domain::PresenceSessionStatus::Active;
            }
        );

        if (match == sessions_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    std::optional<domain::PresenceSession> findActiveByZoneAt(
        const std::string& zoneId,
        std::chrono::system_clock::time_point at
    ) const override {
        const auto match = std::find_if(
            sessions_.begin(),
            sessions_.end(),
            [&zoneId, at](const domain::PresenceSession& session) {
                return session.zoneId == zoneId && session.isActiveAt(at);
            }
        );

        if (match == sessions_.end()) {
            return std::nullopt;
        }

        return *match;
    }

    void create(const domain::PresenceSession& presenceSession) override {
        sessions_.push_back(presenceSession);
    }

    void extend(
        const std::string& sessionId,
        std::chrono::system_clock::time_point expiresAt
    ) override {
        const auto match = find(sessionId);
        if (match != sessions_.end()) {
            match->expiresAt = expiresAt;
        }
    }

    void end(
        const std::string& sessionId,
        std::chrono::system_clock::time_point endedAt
    ) override {
        const auto match = find(sessionId);
        if (match != sessions_.end()) {
            match->endedAt = endedAt;
            match->status = domain::PresenceSessionStatus::Ended;
        }
    }

private:
    std::vector<domain::PresenceSession>::iterator find(const std::string& sessionId) {
        return std::find_if(
            sessions_.begin(),
            sessions_.end(),
            [&sessionId](const domain::PresenceSession& session) {
                return session.sessionId == sessionId;
            }
        );
    }

    std::vector<domain::PresenceSession> sessions_;
};

bool mappingMatches(
    const XProtectZoneEventMapping& mapping,
    const xprotect::XProtectLineCrossingCommand& command
) {
    return mapping.eventName == command.eventName
        && (mapping.sourceName.empty() || mapping.sourceName == command.sourceName);
}

}

struct ApiRuntimeComposition::State {
    explicit State(ApiRuntimeConfig config)
        : config{std::move(config)},
          employees{this->config.employees},
          appUsers{this->config.appUsers},
          zones{this->config.zones},
          presenceSessions{this->config.presenceSessions},
          presenceService{employees, appUsers, zones, qrCheckins, presenceSessions},
          qrService{presenceService, [] { return qr::QrCheckInService::Clock::now(); }, this->config.qrPresenceDuration},
          xprotectService{
              [this](const xprotect::XProtectLineCrossingCommand& command) {
                  return resolveXProtectZone(command);
              },
              [this](const domain::Zone& zone, xprotect::XProtectLineCrossingService::Clock::time_point at) {
                  return presenceSessions.findActiveByZoneAt(zone.zoneId, at);
              }
          } {
        if (this->config.accessTokenService) {
            endpointAuthorizer = std::make_unique<EndpointAuthorizer>(
                *this->config.accessTokenService,
                appUsers,
                this->config.authNowProvider
            );
        }
        if (this->config.passwordVerifier && this->config.accessTokenService) {
            authenticationService = std::make_unique<auth::AuthenticationService>(
                appUsers,
                *this->config.passwordVerifier,
                *this->config.accessTokenService,
                this->config.dummyPasswordHash,
                this->config.authNowProvider
            );
        }
    }

    std::optional<domain::Zone> resolveXProtectZone(
        const xprotect::XProtectLineCrossingCommand& command
    ) const {
        const auto mapping = std::find_if(
            config.xprotectZoneMappings.begin(),
            config.xprotectZoneMappings.end(),
            [&command](const XProtectZoneEventMapping& current) {
                return mappingMatches(current, command);
            }
        );

        if (mapping != config.xprotectZoneMappings.end()) {
            return zones.findActiveByZoneId(mapping->zoneId);
        }

        return zones.findActiveByXProtectEventName(command.eventName);
    }

    ApiRuntimeConfig config;
    InMemoryEmployeeRepository employees;
    InMemoryAppUserRepository appUsers;
    InMemoryZoneRepository zones;
    InMemoryQrCheckinRepository qrCheckins;
    InMemoryPresenceSessionRepository presenceSessions;
    presence::PresenceSessionService presenceService;
    qr::QrCheckInService qrService;
    xprotect::XProtectLineCrossingService xprotectService;
    std::unique_ptr<auth::AuthenticationService> authenticationService;
    std::unique_ptr<EndpointAuthorizer> endpointAuthorizer;
};

ApiRuntimeComposition::ApiRuntimeComposition(ApiRuntimeConfig config)
    : state_{std::make_shared<State>(std::move(config))} {
}

ApiApplication ApiRuntimeComposition::createApplication() const {
    return ApiApplication{
        state_->config.apiSettings,
        state_->config.appInfo,
        createRouteHandlers()
    };
}

ApiRouteHandlers ApiRuntimeComposition::createRouteHandlers() const {
    return ApiRouteHandlers{
        createQrCheckInHandler(),
        createXProtectLineCrossingHandler(),
        {},
        createLoginHandler(),
        createAuthorizationHandler()
    };
}

AuthRoutes::LoginHandler ApiRuntimeComposition::createLoginHandler() const {
    const auto state = state_;
    if (!state->authenticationService) {
        return {};
    }
    return [state](const auth::LoginCommand& command) {
        return state->authenticationService->login(command);
    };
}

EndpointAuthorizer::Handler ApiRuntimeComposition::createAuthorizationHandler() const {
    const auto state = state_;
    if (!state->endpointAuthorizer) {
        return {};
    }
    return [state](const HttpRequest& request, auth::AuthorizationPolicy policy) {
        return state->endpointAuthorizer->authorize(request, policy);
    };
}

QrRoutes::CheckInHandler ApiRuntimeComposition::createQrCheckInHandler() const {
    const auto state = state_;
    return [state](const qr::QrCheckInCommand& command) {
        return state->qrService.checkIn(command);
    };
}

XProtectEventRoutes::LineCrossingHandler
ApiRuntimeComposition::createXProtectLineCrossingHandler() const {
    const auto state = state_;
    return XProtectLineCrossingHandler{state->xprotectService};
}

ApiApplication createComposedApiApplication(ApiRuntimeConfig config) {
    return ApiRuntimeComposition{std::move(config)}.createApplication();
}

}
