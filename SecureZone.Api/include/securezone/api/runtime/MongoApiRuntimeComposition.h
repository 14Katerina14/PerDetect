#pragma once

#include "securezone/api/ApiApplication.h"
#include "securezone/api/runtime/ApiRuntimeComposition.h"
#include "securezone/infrastructure/mongodb/MongoDbSettings.h"

#include <memory>

namespace securezone::api {

struct MongoApiRuntimeConfig {
    ApiRuntimeConfig apiRuntime{};
    infrastructure::mongodb::MongoDbSettings mongoDb{};
};

class MongoApiRuntimeComposition {
public:
    explicit MongoApiRuntimeComposition(MongoApiRuntimeConfig config);

    ApiApplication createApplication() const;
    ApiRouteHandlers createRouteHandlers() const;
    QrRoutes::CheckInHandler createQrCheckInHandler() const;
    AuthRoutes::LoginHandler createLoginHandler() const;
    EndpointAuthorizer::Handler createAuthorizationHandler() const;
    XProtectEventRoutes::LineCrossingHandler createXProtectLineCrossingHandler() const;
    CameraObjectRoutes::ObservationHandler createCameraObjectObservationHandler() const;

private:
    struct State;

    std::shared_ptr<State> state_;
};

MongoApiRuntimeConfig loadMongoApiRuntimeConfigFromEnvironment();
ApiApplication createMongoApiApplicationFromEnvironment();

}
