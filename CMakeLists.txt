cmake_minimum_required(VERSION 3.1)

#set (CMAKE_C_COMPILER "/usr/local/bin/gcc-9")
#set (CMAKE_CXX_COMPILER "/usr/local/bin/g++-9")

project(blackbird)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(cotire)

set (OPENSSL_ROOT_DIR "/usr/local/openssl-1.1.1k")
set (OPENSSL_CRYPTO_LIBRARY "/usr/local/openssl-1.1.1k/lib/libcrypto.dylib")
set (OPENSSL_SSL_LIBRARY "/usr/local/openssl-1.1.1k/lib/libssl.1.1.dylib")
set (OPENSSL_INCLUDE_DIR "/usr/local/openssl-1.1.1k/include")

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

find_package(CURL)
find_package(Jansson)
find_package(OpenSSL)
find_package(SQLite3)

file(GLOB SOURCES
    src/*.cpp
    src/exchanges/binance.cpp
    src/exchanges/kraken.cpp
    src/utils/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -pedantic -Wno-missing-braces)
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
                                      CXX_STANDARD 11
                                      CXX_STANDARD_REQUIRED YES
                                      CXX_EXTENSIONS NO)
target_include_directories (${PROJECT_NAME} PRIVATE
                            ${PROJECT_SOURCE_DIR}/src
                            ${SQLITE3_INCLUDE_DIR}
                            ${CURL_INCLUDE_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${JANSSON_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES}
                                      ${JANSSON_LIBRARIES}
                                      ${OPENSSL_LIBRARIES}
                                      ${SQLITE3_LIBRARIES})
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/..)
cotire(${PROJECT_NAME})
