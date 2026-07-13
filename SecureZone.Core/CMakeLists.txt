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
    src/identity/TrackAssociationService.cpp)
target_compile_features(SecureZoneCore PUBLIC cxx_std_17)
target_include_directories(SecureZoneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SECUREZONE_CORE_TESTS
    ZoneGeometryTests
    AccessPolicyEvaluatorTests
    TrackAssociationTests
    AlarmLifecycleTests
    AlarmPersistenceTests
    RolePermissionTests
)

foreach(test_name IN LISTS SECUREZONE_CORE_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE SecureZoneCore)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

add_executable(DecisionEngineTests tests/decision/DecisionEngineTests.cpp)
target_link_libraries(DecisionEngineTests PRIVATE SecureZoneCore)
add_test(NAME DecisionEngineTests COMMAND DecisionEngineTests)
