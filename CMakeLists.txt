cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("Setting default build type to Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -pthread -Wall -Wno-unused-parameter")

project(MyWebServer CXX)

include_directories(${PROJECT_SOURCE_DIR}/log)
include_directories(${PROJECT_SOURCE_DIR}/pool)


set(SRC ${PROJECT_SOURCE_DIR}/pool/threadpool.h 
        ${PROJECT_SOURCE_DIR}/log/log.cpp
        ${PROJECT_SOURCE_DIR}/log/log.h 
        ${PROJECT_SOURCE_DIR}/log/block_queue.h
        ${PROJECT_SOURCE_DIR}/buffer/buffer.h)


add_subdirectory(./timer)

add_subdirectory(./log)

add_subdirectory(./pool)

add_subdirectory(./http)