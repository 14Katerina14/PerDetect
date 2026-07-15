cmake_minimum_required(VERSION 3.24)

project(PerDetect VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

option(SECUREZONE_BUILD_MONGODB_INFRA "Build MongoDB infrastructure components" ON)

add_subdirectory(SecureZone.Core)
add_subdirectory(SecureZone.Api)
add_subdirectory(SecureZone.Tools)

if(SECUREZONE_BUILD_MONGODB_INFRA)
    add_subdirectory(SecureZone.Infrastructure)
endif()
