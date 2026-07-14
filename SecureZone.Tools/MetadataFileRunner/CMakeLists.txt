cmake_minimum_required(VERSION 3.20)
project(SecureZoneMetadataFileRunner LANGUAGES CXX)

add_executable(SecureZoneMetadataFileRunner
    src/main.cpp)

target_compile_features(SecureZoneMetadataFileRunner PRIVATE cxx_std_17)
target_link_libraries(SecureZoneMetadataFileRunner PRIVATE SecureZoneCore)

add_test(
    NAME MetadataFileRunnerIntegrationTests
    COMMAND
        "${CMAKE_COMMAND}"
        "-DRUNNER=$<TARGET_FILE:SecureZoneMetadataFileRunner>"
        "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-output"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/MetadataFileRunnerIntegrationTests.cmake")
