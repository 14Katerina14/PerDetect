cmake_minimum_required(VERSION 3.20)
project(SecureZoneTools LANGUAGES CXX)

if(TARGET SecureZone.Infrastructure)
    add_executable(
        SecureZone.PasswordHash
        src/PasswordHashMain.cpp
    )
    target_compile_features(SecureZone.PasswordHash PRIVATE cxx_std_20)
    target_link_libraries(SecureZone.PasswordHash PRIVATE SecureZone.Infrastructure)
    securezone_copy_mongodb_runtime(SecureZone.PasswordHash)
endif()
