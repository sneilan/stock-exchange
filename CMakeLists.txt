cmake_minimum_required(VERSION 3.15...3.25)
project(StockExchange)

find_package(spdlog REQUIRED)
find_package(Catch2 REQUIRED)
find_package(SQLite3 REQUIRED)

# Including sodium with cmake using this magic hack.
# https://github.com/robinlinden/libsodium-cmake/blob/master/README.md
include(FetchContent)

# Update the commit to point to whatever libsodium-cmake-commit you want to target.
FetchContent_Declare(Sodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
    GIT_TAG 99f14233eab1d4f7f49c2af4ec836f2e701c445e # HEAD as of 2022-05-28
)
set(SODIUM_DISABLE_TESTS ON)
FetchContent_MakeAvailable(Sodium)

include(CMakePrintHelpers)
# include_directories(${SQLite3_INCLUDE_DIRS})

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

file(GLOB ORDER_BOOK_SOURCES "src/order_book/*.cpp")
file(GLOB USER_SERVICE_SOURCES "src/user_service/*.cpp")
file(GLOB UTIL_SOURCES "src/util/*.cpp")
file(GLOB GATEWAY_SOURCES "src/gateway/*.cpp")
file(GLOB TEST_FILES_1 "tests/*cpp")
file(GLOB TEST_FILES_2 "tests/util/*cpp")
file(GLOB TEST_FILES_3 "tests/order_book/*cpp")
file(GLOB TEST_FILES_4 "tests/user_service/*cpp")

add_executable(main src/main.cpp src/eventstore/eventstore.cpp ${ORDER_BOOK_SOURCES} ${GATEWAY_SOURCES} ${USER_SERVICE_SOURCES} ${UTIL_SOURCES} ${spdlog_DIR})
add_executable(test ${TEST_FILES_1} ${TEST_FILES_2} ${TEST_FILES_3} ${TEST_FILES_4} src/eventstore/eventstore.cpp ${ORDER_BOOK_SOURCES} ${UTIL_SOURCES} ${USER_SERVICE_SOURCES} ${GATEWAY_SOURCES} ${spdlog_DIR})

target_link_libraries(main PRIVATE spdlog::spdlog_header_only)
target_link_libraries(main PRIVATE librt.so)
target_link_libraries(main PRIVATE sodium)
target_link_libraries(main PRIVATE sqlite3)

target_link_libraries(test PRIVATE spdlog::spdlog_header_only)
target_link_libraries(test PRIVATE Catch2::Catch2)
target_link_libraries(test PRIVATE librt.so)
target_link_libraries(test PRIVATE sodium)
target_link_libraries(test PRIVATE sqlite3)
