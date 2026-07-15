#include "securezone/api/runtime/MongoApiRuntimeComposition.h"

#include "securezone/api/handlers/XProtectLineCrossingHandler.h"
#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/MongoDbSettingsProvider.h"
#include "securezone/infrastructure/mongodb/MongoRepositoryProvider.h"
#include "securezone/presence/PresenceSessionService.h"
#include "securezone/qr/QrCheckInService.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace securezone::api {
namespace {

bool mappingMatches(
    const XProtectZoneEventMapping& mapping,
    const xprotect::XProtectLineCrossingCommand& command
) {
    return mapping.eventName == command.eventName
        && (mapping.sourceName.empty() || mapping.sourceName == command.sourceName);
}

infrastructure::mongodb::MongoDbSettings mongoSettingsFromApiSettings(
    const ApiSettings& settings
) {
    auto mongoSettings = infrastructure::mongodb::loadMongoDbSettingsFromEnvironment();
    if (!settings.mongoConnectionString.empty()) {
        mongoSettings.connectionUri = settings.mongoConnectionString;
    }
    if (!settings.mongoDatabaseName.empty()) {
        mongoSettings.databaseName = settings.mongoDatabaseName;
    }

    return mongoSettings;
}

}

struct MongoApiRuntimeComposition::State {
    explicit State(MongoApiRuntimeConfig config)
        : config{std::move(config)},
          mongoClient{this->config.mongoDb},
          repositoryProvider{mongoClient},
          employees{repositoryProvider.employeeRepository()},
          appUsers{repositoryProvider.appUserRepository()},
          zones{repositoryProvider.zoneRepository()},
          qrCheckins{repositoryProvider.qrCheckinRepository()},
          presenceSessions{repositoryProvider.presenceSessionRepository()},
          presenceService{employees, appUsers, zones, qrCheckins, presenceSessions},
          qrService{
              presenceService,
              [] { return qr::QrCheckInService::Clock::now(); },
              this->config.apiRuntime.qrPresenceDuration
          },
          xprotectService{
              [this](const xprotect::XProtectLineCrossingCommand& command) {
                  return resolveXProtectZone(command);
              },
              [this](const domain::Zone& zone, xprotect::XProtectLineCrossingService::Clock::time_point at) {
                  return presenceSessions.findActiveByZoneAt(zone.zoneId, at);
              }
          } {
    }

    std::optional<domain::Zone> resolveXProtectZone(
        const xprotect::XProtectLineCrossingCommand& command
    ) const {
        const auto mapping = std::find_if(
            config.apiRuntime.xprotectZoneMappings.begin(),
            config.apiRuntime.xprotectZoneMappings.end(),
            [&command](const XProtectZoneEventMapping& current) {
                return mappingMatches(current, command);
            }
        );

        if (mapping != config.apiRuntime.xprotectZoneMappings.end()) {
            return zones.findActiveByZoneId(mapping->zoneId);
        }

        return zones.findActiveByXProtectEventName(command.eventName);
    }

    MongoApiRuntimeConfig config;
    infrastructure::mongodb::MongoDbClient mongoClient;
    infrastructure::mongodb::MongoRepositoryProvider repositoryProvider;
    infrastructure::mongodb::repositories::MongoEmployeeRepository employees;
    infrastructure::mongodb::repositories::MongoAppUserRepository appUsers;
    infrastructure::mongodb::repositories::MongoZoneRepository zones;
    infrastructure::mongodb::repositories::MongoQrCheckinRepository qrCheckins;
    infrastructure::mongodb::repositories::MongoPresenceSessionRepository presenceSessions;
    presence::PresenceSessionService presenceService;
    qr::QrCheckInService qrService;
    xprotect::XProtectLineCrossingService xprotectService;
};

MongoApiRuntimeComposition::MongoApiRuntimeComposition(MongoApiRuntimeConfig config)
    : state_{std::make_shared<State>(std::move(config))} {
}

ApiApplication MongoApiRuntimeComposition::createApplication() const {
    return ApiApplication{
        state_->config.apiRuntime.apiSettings,
        state_->config.apiRuntime.appInfo,
        createRouteHandlers()
    };
}

ApiRouteHandlers MongoApiRuntimeComposition::createRouteHandlers() const {
    return ApiRouteHandlers{
        createQrCheckInHandler(),
        createXProtectLineCrossingHandler()
    };
}

QrRoutes::CheckInHandler MongoApiRuntimeComposition::createQrCheckInHandler() const {
    const auto state = state_;
    return [state](const qr::QrCheckInCommand& command) {
        return state->qrService.checkIn(command);
    };
}

XProtectEventRoutes::LineCrossingHandler
MongoApiRuntimeComposition::createXProtectLineCrossingHandler() const {
    const auto state = state_;
    return XProtectLineCrossingHandler{state->xprotectService};
}

MongoApiRuntimeConfig loadMongoApiRuntimeConfigFromEnvironment() {
    MongoApiRuntimeConfig config{};
    config.apiRuntime.apiSettings = loadApiSettingsFromEnvironment();
    config.apiRuntime.qrPresenceDuration = config.apiRuntime.apiSettings.qrPresenceDuration;
    config.mongoDb = mongoSettingsFromApiSettings(config.apiRuntime.apiSettings);
    return config;
}

ApiApplication createMongoApiApplicationFromEnvironment() {
    return MongoApiRuntimeComposition{
        loadMongoApiRuntimeConfigFromEnvironment()
    }.createApplication();
}

}
