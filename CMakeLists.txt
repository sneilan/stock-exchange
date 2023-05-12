cmake_minimum_required(VERSION 3.15...3.25)
project(StockExchange)

find_package(Catch2 REQUIRED)
include(Catch)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

file(GLOB SOURCES "order_book/*.cpp")

add_executable(main main.cpp eventstore.cpp ${SOURCES})




