cmake_minimum_required(VERSION 3.20)
project(SecureZoneApi LANGUAGES CXX)

add_library(SecureZoneApi
    src/ApiApplication.cpp
    src/ApiResponse.cpp
    src/ApiServer.cpp
    src/ApiSettings.cpp
    src/http/Router.cpp
    src/routes/HealthRoutes.cpp
    src/routes/QrRoutes.cpp
    src/routes/XProtectEventRoutes.cpp
)

target_compile_features(SecureZoneApi PUBLIC cxx_std_17)
target_include_directories(SecureZoneApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SecureZoneApi PUBLIC SecureZoneCore)

add_executable(SecureZone.Api src/main.cpp)
target_link_libraries(SecureZone.Api PRIVATE SecureZoneApi)

add_executable(ApiSkeletonTests tests/ApiSkeletonTests.cpp)
target_link_libraries(ApiSkeletonTests PRIVATE SecureZoneApi)
add_test(NAME ApiSkeletonTests COMMAND ApiSkeletonTests)
