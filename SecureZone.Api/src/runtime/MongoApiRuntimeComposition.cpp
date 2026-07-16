#include "securezone/api/runtime/MongoApiRuntimeComposition.h"

#include "securezone/api/handlers/XProtectLineCrossingHandler.h"
#include "securezone/api/auth/EndpointAuthorizer.h"
#include "securezone/auth/AuthenticationService.h"
#include "securezone/infrastructure/auth/JwtAccessTokenService.h"
#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"
#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/MongoDbSettingsProvider.h"
#include "securezone/infrastructure/mongodb/MongoRepositoryProvider.h"
#include "securezone/presence/PresenceSessionService.h"
#include "securezone/notification/PushNotificationService.h"
#include "securezone/identity/CameraIdentityService.h"
#include "securezone/identity/UnidentifiedPersonWatchdog.h"
#include "securezone/qr/QrCheckInService.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"
#include "securezone/xprotect/XProtectPolicyAlarmService.h"

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
          machines{repositoryProvider.machineRepository()},
          accessPolicies{repositoryProvider.accessPolicyRepository()},
          alarms{repositoryProvider.alarmRepository()},
          qrCheckins{repositoryProvider.qrCheckinRepository()},
          presenceSessions{repositoryProvider.presenceSessionRepository()},
          cameraObjectTracks{repositoryProvider.cameraObjectTrackRepository()},
          trackIdentityBindings{repositoryProvider.trackIdentityBindingRepository()},
          pushSubscriptions{repositoryProvider.pushSubscriptionRepository()},
          pushDeliveries{repositoryProvider.pushNotificationDeliveryRepository()},
          passwordVerifier{},
          dummyPasswordHash{infrastructure::auth::hashPasswordArgon2id(
              "securezone-dummy-password-verification-only"
          )},
          accessTokens{{
              this->config.apiRuntime.apiSettings.jwtSecret,
              this->config.apiRuntime.apiSettings.jwtTtl,
              "securezone",
              "securezone-mobile"
          }},
          authenticationService{appUsers, passwordVerifier, accessTokens, dummyPasswordHash},
          endpointAuthorizer{accessTokens, appUsers},
          cameraIdentityService{cameraObjectTracks, trackIdentityBindings},
          presenceService{employees, appUsers, zones, qrCheckins, presenceSessions},
          qrService{
              presenceService,
              cameraIdentityService,
              [] { return qr::QrCheckInService::Clock::now(); },
              this->config.apiRuntime.qrPresenceDuration
          },
          pushNotificationService{pushSubscriptions, pushDeliveries},
          unidentifiedPersonWatchdog{
              cameraIdentityService,
              cameraObjectTracks,
              zones,
              alarms,
              [this](const domain::Alarm& alarm) {
                  pushNotificationService.queueAlarmNotifications(
                      alarm,
                      std::chrono::system_clock::now()
                  );
              },
              this->config.apiRuntime.apiSettings.unidentifiedIdentityGracePeriod
          },
          policyAlarmService{
              employees,
              accessPolicies,
              machines,
              alarms,
              [this](const domain::Alarm& alarm) {
                  pushNotificationService.queueAlarmNotifications(
                      alarm,
                      std::chrono::system_clock::now()
                  );
              },
              [this](const domain::Alarm& alarm) {
                  pushNotificationService.queueAlarmResolvedNotifications(
                      alarm,
                      std::chrono::system_clock::now()
                  );
              }
          },
          xprotectService{
              [this](const xprotect::XProtectLineCrossingCommand& command) {
                  return resolveXProtectZone(command);
              },
              [this](const std::string& cameraId, const std::string& objectId,
                     xprotect::XProtectLineCrossingService::Clock::time_point at) {
                  return cameraIdentityService.resolve(cameraId, objectId, at);
              },
              [this](
                  const xprotect::XProtectLineCrossingCommand& command,
                  const domain::Zone& zone,
                  const std::optional<domain::TrackIdentityBinding>& binding
              ) {
                  return policyAlarmService.evaluate(command, zone, binding);
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
    infrastructure::mongodb::repositories::MongoMachineRepository machines;
    infrastructure::mongodb::repositories::MongoAccessPolicyRepository accessPolicies;
    infrastructure::mongodb::repositories::MongoAlarmRepository alarms;
    infrastructure::mongodb::repositories::MongoQrCheckinRepository qrCheckins;
    infrastructure::mongodb::repositories::MongoPresenceSessionRepository presenceSessions;
    infrastructure::mongodb::repositories::MongoCameraObjectTrackRepository cameraObjectTracks;
    infrastructure::mongodb::repositories::MongoTrackIdentityBindingRepository trackIdentityBindings;
    infrastructure::mongodb::repositories::MongoPushSubscriptionRepository pushSubscriptions;
    infrastructure::mongodb::repositories::MongoPushNotificationDeliveryRepository pushDeliveries;
    infrastructure::auth::SodiumPasswordVerifier passwordVerifier;
    std::string dummyPasswordHash;
    infrastructure::auth::JwtAccessTokenService accessTokens;
    auth::AuthenticationService authenticationService;
    EndpointAuthorizer endpointAuthorizer;
    identity::CameraIdentityService cameraIdentityService;
    presence::PresenceSessionService presenceService;
    qr::QrCheckInService qrService;
    notification::PushNotificationService pushNotificationService;
    identity::UnidentifiedPersonWatchdog unidentifiedPersonWatchdog;
    xprotect::XProtectPolicyAlarmService policyAlarmService;
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
        createXProtectLineCrossingHandler(),
        createCameraObjectObservationHandler(),
        createLoginHandler(),
        createAuthorizationHandler()
    };
}

AuthRoutes::LoginHandler MongoApiRuntimeComposition::createLoginHandler() const {
    const auto state = state_;
    return [state](const auth::LoginCommand& command) {
        return state->authenticationService.login(command);
    };
}

EndpointAuthorizer::Handler MongoApiRuntimeComposition::createAuthorizationHandler() const {
    const auto state = state_;
    return [state](const HttpRequest& request, auth::AuthorizationPolicy policy) {
        return state->endpointAuthorizer.authorize(request, policy);
    };
}

CameraObjectRoutes::ObservationHandler
MongoApiRuntimeComposition::createCameraObjectObservationHandler() const {
    const auto state = state_;
    return [state](const identity::CameraObjectObservation& observation) {
        return state->unidentifiedPersonWatchdog.observe(observation);
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
    config.apiRuntime.appInfo = loadApiApplicationInfoFromEnvironment();
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
