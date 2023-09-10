cmake_minimum_required(VERSION 3.15...3.25)
project(StockExchange)

find_package(spdlog REQUIRED)
find_package(Catch2 REQUIRED)

include(CMakePrintHelpers)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

file(GLOB ORDER_BOOK_SOURCES "order_book/*.cpp")
file(GLOB UTIL_SOURCES "util/*.cpp")
file(GLOB GATEWAY_SOURCES "gateway/*.cpp")
file(GLOB TEST_FILES_1 "tests/*cpp")
file(GLOB TEST_FILES_2 "tests/util/*cpp")
file(GLOB TEST_FILES_3 "tests/order_book/*cpp")

add_executable(main main.cpp eventstore.cpp ${ORDER_BOOK_SOURCES} ${GATEWAY_SOURCES} ${UTIL_SOURCES} ${spdlog_DIR})
add_executable(test ${TEST_FILES_1} ${TEST_FILES_2} ${TEST_FILES_3} eventstore.cpp ${ORDER_BOOK_SOURCES} ${UTIL_SOURCES} ${GATEWAY_SOURCES} ${spdlog_DIR})

target_link_libraries(main PRIVATE spdlog::spdlog_header_only)
target_link_libraries(main PRIVATE librt.so)
target_link_libraries(test PRIVATE spdlog::spdlog_header_only)
target_link_libraries(test PRIVATE Catch2::Catch2)
target_link_libraries(test PRIVATE librt.so)
