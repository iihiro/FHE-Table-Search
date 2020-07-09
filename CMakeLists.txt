cmake_minimum_required(VERSION 3.0.0)

set(project_name FHE-Table-Search)
project(${project_name})

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "-O3 -std=c++17 -openmp -pthread -Wall -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -std=c++17 -openmp -pthread -Wall")

include_directories(${PROJECT_SOURCE_DIR}/fts ${PROJECT_SOURCE_DIR}/stdsc)
#link_directories(${CMAKE_LIB_PALISADE})

set(COMMON_LIBS stdsc fts_share)

add_subdirectory(stdsc)
add_subdirectory(fts)
add_subdirectory(demo)