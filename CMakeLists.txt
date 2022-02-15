cmake_minimum_required(VERSION 3.10)

project(MCPT)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DDEBUG)

include_directories(include)

set(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-g -O2 -rdynamic -w -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O2 -rdynamic -w -DNDEBUG")

SET(CMAKE_BUILD_TYPE "Release")

add_subdirectory(src)

add_subdirectory(include)

add_executable(mcpt src/main.cpp)

target_link_libraries(mcpt wheels tinyxml2)