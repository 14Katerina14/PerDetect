cmake_minimum_required(VERSION 3.20)
project(SecureZoneApi LANGUAGES CXX)

add_library(SecureZoneApi
    src/ApiApplication.cpp
    src/ApiResponse.cpp
    src/ApiServer.cpp
    src/ApiSettings.cpp
    src/events/XProtectLineCrossingEvent.cpp
    src/handlers/XProtectLineCrossingHandler.cpp
    src/http/Router.cpp
    src/routes/HealthRoutes.cpp
    src/routes/QrRoutes.cpp
    src/routes/XProtectEventRoutes.cpp
    src/runtime/ApiRuntimeComposition.cpp
)

target_compile_features(SecureZoneApi PUBLIC cxx_std_17)
target_include_directories(SecureZoneApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SecureZoneApi PUBLIC SecureZoneCore)

if(TARGET SecureZone.Infrastructure)
    target_sources(SecureZoneApi PRIVATE src/runtime/MongoApiRuntimeComposition.cpp)
    target_link_libraries(SecureZoneApi PRIVATE SecureZone.Infrastructure)
    target_compile_definitions(SecureZoneApi PRIVATE SECUREZONE_WITH_MONGODB_INFRA=1)
endif()

add_executable(SecureZone.Api src/main.cpp)
target_link_libraries(SecureZone.Api PRIVATE SecureZoneApi)

add_executable(ApiSkeletonTests tests/ApiSkeletonTests.cpp)
target_link_libraries(ApiSkeletonTests PRIVATE SecureZoneApi)
add_test(NAME ApiSkeletonTests COMMAND ApiSkeletonTests)

if(TARGET SecureZone.Infrastructure AND WIN32)
    if(DEFINED bsoncxx_DIR)
        get_filename_component(
            SECUREZONE_MONGODB_PACKAGE_ROOT
            "${bsoncxx_DIR}/../.."
            ABSOLUTE
        )
        set(SECUREZONE_VCPKG_RUNTIME_PATH
            "${SECUREZONE_MONGODB_PACKAGE_ROOT}/$<IF:$<CONFIG:Debug>,debug/bin,bin>"
        )
        set_tests_properties(
            ApiSkeletonTests
            PROPERTIES
                ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${SECUREZONE_VCPKG_RUNTIME_PATH}"
        )
    endif()

    foreach(securezone_runtime_target SecureZone.Api ApiSkeletonTests)
        add_custom_command(
            TARGET ${securezone_runtime_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${securezone_runtime_target}>
                $<TARGET_FILE_DIR:${securezone_runtime_target}>
            COMMAND_EXPAND_LISTS
        )

        if(DEFINED SECUREZONE_VCPKG_RUNTIME_PATH)
            add_custom_command(
                TARGET ${securezone_runtime_target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${SECUREZONE_VCPKG_RUNTIME_PATH}"
                    $<TARGET_FILE_DIR:${securezone_runtime_target}>
            )
        endif()
    endforeach()
endif()
