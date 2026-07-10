cmake_minimum_required(VERSION 3.20)
project(SecureZoneCore LANGUAGES CXX)

add_library(SecureZoneCore)
target_compile_features(SecureZoneCore PUBLIC cxx_std_17)
target_include_directories(SecureZoneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
