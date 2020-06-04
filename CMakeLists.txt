cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(addmc)
file(GLOB SOURCES "src/implementation/*.cpp")

include_directories(src/interface)

include_directories(lib)
include_directories(lib/cudd-3.0.0)
include_directories(lib/cudd-3.0.0/cudd)
include_directories(lib/cudd-3.0.0/epd)
include_directories(lib/cudd-3.0.0/mtr)
include_directories(lib/cudd-3.0.0/st)

link_directories(lib/cudd-3.0.0/cudd/.libs)
add_executable(addmc ${SOURCES})
target_link_libraries(addmc cudd)
