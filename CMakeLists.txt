cmake_minimum_required(VERSION 4.4)
project(WXClient)

set(CMAKE_CXX_STANDARD 20)

add_executable(WXClient
        src/main.cpp
        src/ControllerAPI.cpp
)

target_include_directories(WXClient PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

find_package(
        OpenSSL REQUIRED
)
target_link_libraries(
        WXClient PRIVATE OpenSSL::SSL OpenSSL::Crypto
)
target_compile_definitions(
        WXClient PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT
)