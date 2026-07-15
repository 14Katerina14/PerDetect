find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)

add_library(SecureZone.Infrastructure
    src/mongodb/MongoDbClient.cpp
    src/mongodb/MongoDocumentMappers.cpp
    src/mongodb/MongoRepositoryProvider.cpp
    src/mongodb/MongoDbSettingsProvider.cpp
    src/mongodb/repositories/MongoAccessPolicyRepository.cpp
    src/mongodb/repositories/MongoAlarmRepository.cpp
    src/mongodb/repositories/MongoAppUserRepository.cpp
    src/mongodb/repositories/MongoEmployeeRepository.cpp
    src/mongodb/repositories/MongoMachineRepository.cpp
    src/mongodb/repositories/MongoPresenceSessionRepository.cpp
    src/mongodb/repositories/MongoQrCheckinRepository.cpp
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
        mongo::bsoncxx_shared
        mongo::mongocxx_shared
)

target_compile_features(SecureZone.Infrastructure PUBLIC cxx_std_20)

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
        SKIP_RETURN_CODE 77
)
