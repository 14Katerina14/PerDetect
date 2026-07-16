cmake_minimum_required(VERSION 3.20)
project(SecureZoneApi LANGUAGES CXX)

find_package(nlohmann_json CONFIG QUIET)
if(NOT TARGET nlohmann_json::nlohmann_json)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

add_library(SecureZoneApi
    src/auth/EndpointAuthorizer.cpp
    src/ApiApplication.cpp
    src/ApiResponse.cpp
    src/ApiServer.cpp
    src/ApiSettings.cpp
    src/events/XProtectLineCrossingEvent.cpp
    src/handlers/XProtectLineCrossingHandler.cpp
    src/http/Router.cpp
    src/routes/HealthRoutes.cpp
    src/routes/AuthRoutes.cpp
    src/routes/CameraObjectRoutes.cpp
    src/routes/QrRoutes.cpp
    src/routes/XProtectEventRoutes.cpp
    src/runtime/ApiRuntimeComposition.cpp
)

target_compile_features(SecureZoneApi PUBLIC cxx_std_17)
target_include_directories(SecureZoneApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SecureZoneApi PUBLIC SecureZoneCore nlohmann_json::nlohmann_json)

if(SECUREZONE_BUILD_HTTP_RUNTIME)
    find_package(httplib CONFIG QUIET)
    if(NOT TARGET httplib::httplib)
        include(FetchContent)
        set(HTTPLIB_INSTALL OFF CACHE BOOL "" FORCE)
        set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
        set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
        set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
        set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            cpp_httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG v0.36.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(cpp_httplib)
    endif()
    target_sources(SecureZoneApi PRIVATE src/http/HttpRuntimeServer.cpp)
    target_link_libraries(SecureZoneApi PRIVATE httplib::httplib)
endif()

if(TARGET SecureZone.Infrastructure)
    target_sources(SecureZoneApi PRIVATE src/runtime/MongoApiRuntimeComposition.cpp)
    target_link_libraries(SecureZoneApi PRIVATE SecureZone.Infrastructure)
    target_compile_definitions(SecureZoneApi PRIVATE SECUREZONE_WITH_MONGODB_INFRA=1)
endif()

add_executable(SecureZone.Api src/main.cpp)
target_link_libraries(SecureZone.Api PRIVATE SecureZoneApi)

if(SECUREZONE_BUILD_HTTP_RUNTIME)
    target_compile_definitions(SecureZone.Api PRIVATE SECUREZONE_WITH_HTTP_RUNTIME=1)
endif()

add_executable(ApiSkeletonTests tests/ApiSkeletonTests.cpp)
target_link_libraries(ApiSkeletonTests PRIVATE SecureZoneApi)
add_test(NAME ApiSkeletonTests COMMAND ApiSkeletonTests)

add_executable(AuthSecurityTests tests/AuthSecurityTests.cpp)
target_link_libraries(AuthSecurityTests PRIVATE SecureZoneApi)
add_test(NAME AuthSecurityTests COMMAND AuthSecurityTests)

if(TARGET SecureZone.Infrastructure AND WIN32)
    foreach(securezone_runtime_target SecureZone.Api ApiSkeletonTests AuthSecurityTests)
        securezone_copy_mongodb_runtime(${securezone_runtime_target})
    endforeach()
    securezone_set_mongodb_test_runtime(ApiSkeletonTests)
    securezone_set_mongodb_test_runtime(AuthSecurityTests)
endif()
