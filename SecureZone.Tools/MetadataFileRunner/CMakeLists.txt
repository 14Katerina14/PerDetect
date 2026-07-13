cmake_minimum_required(VERSION 3.20)
project(SecureZoneMetadataFileRunner LANGUAGES CXX)

add_executable(SecureZoneMetadataFileRunner
    src/main.cpp)

target_compile_features(SecureZoneMetadataFileRunner PRIVATE cxx_std_17)
target_link_libraries(SecureZoneMetadataFileRunner PRIVATE SecureZoneCore)
