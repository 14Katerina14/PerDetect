cmake_minimum_required(VERSION 3.20)
project(SecureZoneCore LANGUAGES CXX)

add_library(SecureZoneCore
    src/alarm/AlarmLifecycleService.cpp
    src/alarm/AlarmPersistenceService.cpp
    src/decision/AccessPolicyEvaluator.cpp
    src/decision/DecisionContextLoader.cpp
    src/decision/DecisionEngine.cpp
    src/geometry/ZoneGeometryService.cpp
    src/identity/RolePermissionService.cpp
    src/identity/TrackAssociationService.cpp
    src/machine/MachineStatusService.cpp
    src/metadata/MetadataApplicationService.cpp
    src/metadata/MetadataDecisionTriggerService.cpp
    src/metadata/MetadataIngestionService.cpp
    src/metadata/MetadataPersistenceService.cpp
    src/metadata/MetadataProcessingService.cpp
    src/metadata/OnvifMetadataParser.cpp
    src/presence/PresenceSessionService.cpp)
target_compile_features(SecureZoneCore PUBLIC cxx_std_17)
target_include_directories(SecureZoneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SECUREZONE_CORE_TESTS
    ZoneGeometryTests
    AccessPolicyEvaluatorTests
    TrackAssociationTests
    AlarmLifecycleTests
    AlarmPersistenceTests
    RolePermissionTests
    MachineStatusServiceTests
    MetadataApplicationServiceTests
    MetadataDecisionTriggerServiceTests
    MetadataIngestionServiceTests
    MetadataPersistenceServiceTests
    MetadataProcessingServiceTests
    OnvifMetadataParserTests
    PresenceSessionServiceTests
)

foreach(test_name IN LISTS SECUREZONE_CORE_TESTS)
    if(test_name STREQUAL "PresenceSessionServiceTests")
        set(test_source tests/presence/PresenceSessionServiceTests.cpp)
    elseif(test_name STREQUAL "MetadataApplicationServiceTests")
        set(test_source tests/metadata/MetadataApplicationServiceTests.cpp)
    elseif(test_name STREQUAL "MetadataDecisionTriggerServiceTests")
        set(test_source tests/metadata/MetadataDecisionTriggerServiceTests.cpp)
    elseif(test_name STREQUAL "MetadataIngestionServiceTests")
        set(test_source tests/metadata/MetadataIngestionServiceTests.cpp)
    elseif(test_name STREQUAL "MetadataPersistenceServiceTests")
        set(test_source tests/metadata/MetadataPersistenceServiceTests.cpp)
    elseif(test_name STREQUAL "MetadataProcessingServiceTests")
        set(test_source tests/metadata/MetadataProcessingServiceTests.cpp)
    elseif(test_name STREQUAL "OnvifMetadataParserTests")
        set(test_source tests/metadata/OnvifMetadataParserTests.cpp)
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
