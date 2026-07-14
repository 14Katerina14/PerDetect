cmake_minimum_required(VERSION 3.20)
project(SecureZoneBackend LANGUAGES CXX)

add_executable(SecureZoneBackend
    src/BackendApplication.cpp
    src/BackendCompositionRoot.cpp
    src/BackendConfig.cpp
    src/DemoBackendConfiguration.cpp
    src/InMemoryDecisionRepositories.cpp
    src/InMemoryMetadataRepositories.cpp
    src/main.cpp)

target_compile_features(SecureZoneBackend PRIVATE cxx_std_20)
target_include_directories(SecureZoneBackend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(SecureZoneBackend PRIVATE SecureZoneCore)
