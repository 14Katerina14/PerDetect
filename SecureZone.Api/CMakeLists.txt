add_library(SecureZoneQrApi
    src/routes/QrRoutes.cpp
)

add_library(SecureZone::QrApi ALIAS SecureZoneQrApi)

target_include_directories(SecureZoneQrApi
    PUBLIC
        include
)

target_link_libraries(SecureZoneQrApi
    PUBLIC
        SecureZoneCore
)

target_compile_features(SecureZoneQrApi PUBLIC cxx_std_17)

add_executable(QrRoutesTests tests/QrRoutesTests.cpp)
target_link_libraries(QrRoutesTests PRIVATE SecureZoneQrApi)
add_test(NAME QrRoutesTests COMMAND QrRoutesTests)
