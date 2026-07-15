cmake_minimum_required(VERSION 3.20)
project(SecureZoneCore LANGUAGES CXX)

add_library(SecureZoneCore
    src/alarm/AlarmLifecycleService.cpp
    src/configuration/AccessPolicyConfigurationService.cpp
    src/configuration/WebhookTargetConfigurationService.cpp
    src/configuration/ZoneConfigurationService.cpp
    src/alarm/AlarmPersistenceService.cpp
    src/decision/AccessPolicyEvaluator.cpp
    src/decision/DecisionContextLoader.cpp
    src/decision/DecisionEngine.cpp
    src/identity/RolePermissionService.cpp
    src/machine/MachineStatusService.cpp
    src/presence/PresenceSessionService.cpp
    src/webhook/WebhookNotificationService.cpp
    src/webhook/WebhookDeliveryService.cpp)
target_compile_features(SecureZoneCore PUBLIC cxx_std_17)
target_include_directories(SecureZoneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SECUREZONE_CORE_TESTS
    AccessPolicyEvaluatorTests
    AlarmLifecycleTests
    AlarmPersistenceTests
    RolePermissionTests
    MachineStatusServiceTests
    ConfigurationServicesTests
    PresenceSessionServiceTests
    WebhookNotificationServiceTests
    WebhookDeliveryServiceTests
)

foreach(test_name IN LISTS SECUREZONE_CORE_TESTS)
    if(test_name STREQUAL "PresenceSessionServiceTests")
        set(test_source tests/presence/PresenceSessionServiceTests.cpp)
    elseif(test_name STREQUAL "ConfigurationServicesTests")
        set(test_source tests/configuration/ConfigurationServicesTests.cpp)
    else()
        set(test_source tests/${test_name}.cpp)
    endif()
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE SecureZoneCore)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

add_executable(DecisionEngineTests tests/decision/DecisionEngineTests.cpp)
target_link_libraries(DecisionEngineTests PRIVATE SecureZoneCore)
add_test(NAME DecisionEngineTests COMMAND DecisionEngineTests)
