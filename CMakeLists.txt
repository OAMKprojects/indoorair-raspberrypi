cmake_minimum_required(VERSION 3.0.0)
project(indoorair-server VERSION 0.1.0)

find_package (SQLite3 REQUIRED)

file(GLOB SOURCES
    *.hpp
    serial.cpp
    application.cpp
)

add_executable(indoorair-server main.cpp)

include_directories(${SQLite3_INCLUDE_DIRS})

target_link_libraries(indoorair-server ${SOURCES})
target_link_libraries(indoorair-server -lsqlite3)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
