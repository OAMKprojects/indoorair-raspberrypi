cmake_minimum_required(VERSION 3.0.0)
project(indoorair-server VERSION 0.1.0)

find_package (SQLite3 REQUIRED)

file(GLOB SOURCES "src/*.cpp")

option(ADMIN "Admin app" OFF)
if(ADMIN)
    add_definitions(-DADMIN_APP)
endif(ADMIN)

add_executable(indoorair-server ${SOURCES})

include_directories(${SQLite3_INCLUDE_DIRS} src/include)

target_link_libraries(indoorair-server -lsqlite3)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
