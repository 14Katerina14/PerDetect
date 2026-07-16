find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)

add_library(SecureZone.Infrastructure
    src/auth/JwtAccessTokenService.cpp
    src/auth/SodiumPasswordVerifier.cpp
    src/mongodb/MongoDbClient.cpp
    src/mongodb/MongoDocumentMappers.cpp
    src/mongodb/MongoRepositoryProvider.cpp
    src/mongodb/MongoDbSettingsProvider.cpp
    src/mongodb/repositories/MongoAccessPolicyRepository.cpp
    src/mongodb/repositories/MongoCameraObjectTrackRepository.cpp
    src/mongodb/repositories/MongoAlarmRepository.cpp
    src/mongodb/repositories/MongoAppUserRepository.cpp
    src/mongodb/repositories/MongoEmployeeRepository.cpp
    src/mongodb/repositories/MongoMachineRepository.cpp
    src/mongodb/repositories/MongoPresenceSessionRepository.cpp
    src/mongodb/repositories/MongoPushNotificationDeliveryRepository.cpp
    src/mongodb/repositories/MongoPushSubscriptionRepository.cpp
    src/mongodb/repositories/MongoQrCheckinRepository.cpp
    src/mongodb/repositories/MongoTrackIdentityBindingRepository.cpp
    src/mongodb/repositories/MongoWebhookTargetRepository.cpp
    src/mongodb/repositories/MongoZoneRepository.cpp
)

add_library(SecureZone::Infrastructure ALIAS SecureZone.Infrastructure)

target_include_directories(SecureZone.Infrastructure
    PUBLIC
        include
        ../SecureZone.Core/include
)

target_link_libraries(SecureZone.Infrastructure
    PUBLIC
        SecureZoneCore
        jwt-cpp::jwt-cpp
        nlohmann_json::nlohmann_json
        unofficial-sodium::sodium
        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
)

target_compile_features(SecureZone.Infrastructure PUBLIC cxx_std_20)

option(SECUREZONE_BUILD_MONGODB_TESTS "Build MongoDB infrastructure unit tests" ON)
option(SECUREZONE_BUILD_MONGODB_INTEGRATION_TESTS "Build MongoDB integration tests that require a running MongoDB instance" OFF)

if(SECUREZONE_BUILD_MONGODB_TESTS)
add_executable(
    MongoAppUserMapperTests
    tests/mongodb/MongoAppUserMapperTests.cpp
)
target_link_libraries(
    MongoAppUserMapperTests
    PRIVATE
        SecureZone.Infrastructure
)
add_test(NAME MongoAppUserMapperTests COMMAND MongoAppUserMapperTests)
set_tests_properties(
    MongoAppUserMapperTests
    PROPERTIES
        LABELS "mongodb;unit"
)

securezone_copy_mongodb_runtime(MongoAppUserMapperTests)
securezone_set_mongodb_test_runtime(MongoAppUserMapperTests)

add_executable(
    AuthenticationInfrastructureTests
    tests/auth/AuthenticationInfrastructureTests.cpp
)
target_link_libraries(
    AuthenticationInfrastructureTests
    PRIVATE
        SecureZone.Infrastructure
)
add_test(NAME AuthenticationInfrastructureTests COMMAND AuthenticationInfrastructureTests)
set_tests_properties(
    AuthenticationInfrastructureTests
    PROPERTIES
        LABELS "auth;unit"
)
securezone_copy_mongodb_runtime(AuthenticationInfrastructureTests)
securezone_set_mongodb_test_runtime(AuthenticationInfrastructureTests)

add_executable(
    MongoDomainMapperTests
    tests/mongodb/MongoDomainMapperTests.cpp
)
target_link_libraries(
    MongoDomainMapperTests
    PRIVATE
        SecureZone.Infrastructure
)
add_test(NAME MongoDomainMapperTests COMMAND MongoDomainMapperTests)
set_tests_properties(
    MongoDomainMapperTests
    PROPERTIES
        LABELS "mongodb;unit"
)
securezone_copy_mongodb_runtime(MongoDomainMapperTests)
securezone_set_mongodb_test_runtime(MongoDomainMapperTests)
endif()

if(SECUREZONE_BUILD_MONGODB_INTEGRATION_TESTS)
add_executable(
    MongoAppUserRepositoryTests
    tests/mongodb/MongoAppUserRepositoryTests.cpp
)
target_link_libraries(
    MongoAppUserRepositoryTests
    PRIVATE
        SecureZone.Infrastructure
)
add_test(NAME MongoAppUserRepositoryTests COMMAND MongoAppUserRepositoryTests)
set_tests_properties(
    MongoAppUserRepositoryTests
    PROPERTIES
        LABELS "mongodb;integration"
        SKIP_RETURN_CODE 77
)
securezone_copy_mongodb_runtime(MongoAppUserRepositoryTests)
securezone_set_mongodb_test_runtime(MongoAppUserRepositoryTests)
endif()
