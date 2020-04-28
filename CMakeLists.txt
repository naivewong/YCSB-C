cmake_minimum_required(VERSION 2.8)

if("${CMAKE_VERSION}" VERSION_GREATER 3.0.0)
    cmake_policy(SET CMP0042 NEW)
endif()

project("YCSB-C")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -g -Wall -pthread -I./")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -g")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/db")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/redis")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/core")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/db")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/redis")

add_executable(Test1 hashmap_test.cc)
target_link_libraries(Test1 ycsb_core_static ycsb_db_static)