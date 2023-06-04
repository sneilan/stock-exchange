cmake_minimum_required(VERSION 3.15...3.25)
project(StockExchange)

find_package(spdlog REQUIRED)
find_package(Catch2 REQUIRED)
find_package(Protobuf REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

include_directories(${PROTOBUF_INCLUDE_DIRS})

file(GLOB ORDER_BOOK_SOURCES "order_book/*.cpp")
file(GLOB GATEWAY_SOURCES "gateway/*.cpp")
file(GLOB PROTO_SOURCES "gateway/proto/*.cc")
file(GLOB TEST_FILES_1 "tests/*cpp")
file(GLOB TEST_FILES_2 "tests/order_book/*cpp")

add_executable(main main.cpp eventstore.cpp ${ORDER_BOOK_SOURCES} ${GATEWAY_SOURCES} ${PROTO_SOURCES} ${Protobuf_LIBRARY_RELEASE} ${spdlog_DIR})
add_executable(test ${TEST_FILES_1} ${TEST_FILES_2} eventstore.cpp ${ORDER_BOOK_SOURCES} ${GATEWAY_SOURCES} ${PROTO_SOURCES} ${Protobuf_LIBRARY_RELEASE} ${spdlog_DIR})

target_link_libraries(main PRIVATE ${PROTOBUF_LIBRARIES})
target_link_libraries(main PRIVATE spdlog::spdlog_header_only)
target_link_libraries(test PRIVATE ${PROTOBUF_LIBRARIES})
target_link_libraries(test PRIVATE spdlog::spdlog_header_only)
target_link_libraries(test PRIVATE Catch2::Catch2)


