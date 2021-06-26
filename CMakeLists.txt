cmake_minimum_required(VERSION 3.12)

include(Environment.cmake)

project(blackbird)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(cotire)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,--copy-dt-needed-entries")

find_package(CURL)
find_package(Jansson)
find_package(OpenSSL)
find_package(SQLite3)
find_package(Boost 1.76 REQUIRED)
find_package(spdlog REQUIRED)

file(GLOB SOURCES
    src/*.cpp
    src/exchanges/Binance.cpp
    src/exchanges/Kraken.cpp
    src/exchanges/Coinbase.cpp
    src/utils/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -pedantic -Wno-missing-braces)
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()
add_compile_definitions(CCAPI_ENABLE_SERVICE_MARKET_DATA)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_BINANCE)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_KRAKEN)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_COINBASE)

set_target_properties(${PROJECT_NAME} PROPERTIES
                                      CXX_STANDARD 17
                                      CXX_STANDARD_REQUIRED YES
                                      CXX_EXTENSIONS NO)
target_include_directories (${PROJECT_NAME} PRIVATE
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src
                            ${SQLITE3_INCLUDE_DIR}
                            ${Boost_INCLUDE_DIR}
                            ${RAPIDJSON_INCLUDE_DIR}
                            ${WEBSOCKETPP_INCLUDE_DIR}
                            ${CURL_INCLUDE_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${JANSSON_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES}
                                      ${JANSSON_LIBRARIES}
                                      ${OPENSSL_LIBRARIES}
                                      ${SQLITE3_LIBRARIES}
                                      spdlog::spdlog)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/..)
cotire(${PROJECT_NAME})
