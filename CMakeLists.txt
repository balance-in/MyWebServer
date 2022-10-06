cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("Setting default build type to Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -pthread -Wall -Wno-unused-parameter")

project(MyWebServer CXX)

# include_directories(${PROJECT_SOURCE_DIR}/log)
# include_directories(${PROJECT_SOURCE_DIR}/pool)

# aux_source_directory(${PROJECT_SOURCE_DIR}/pool dir-pool)
# aux_source_directory(${PROJECT_SOURCE_DIR}/http dir-http)
# aux_source_directory(${PROJECT_SOURCE_DIR}/log dir-log)
# aux_source_directory(${PROJECT_SOURCE_DIR}/server dir-server)
# aux_source_directory(${PROJECT_SOURCE_DIR}/timer dir-timer)

# include_directories(${PROJECT_SOURCE_DIR}/pool)
# include_directories(${PROJECT_SOURCE_DIR}/http)
# include_directories(${PROJECT_SOURCE_DIR}/log)
# include_directories(${PROJECT_SOURCE_DIR}/server)
# include_directories(${PROJECT_SOURCE_DIR}/timer)



set(SRC ${PROJECT_SOURCE_DIR}/pool/threadpool.h
        ${PROJECT_SOURCE_DIR}/pool/sqlconnpool.cpp
        ${PROJECT_SOURCE_DIR}/pool/sqlconnpool.h
        ${PROJECT_SOURCE_DIR}/http/httpconn.cpp
        ${PROJECT_SOURCE_DIR}/http/httpconn.h
        ${PROJECT_SOURCE_DIR}/http/request.cpp
        ${PROJECT_SOURCE_DIR}/http/request.h
        ${PROJECT_SOURCE_DIR}/http/response.cpp
        ${PROJECT_SOURCE_DIR}/http/response.h
        ${PROJECT_SOURCE_DIR}/server/epoller.cpp
        ${PROJECT_SOURCE_DIR}/server/webserver.cpp
        ${PROJECT_SOURCE_DIR}/log/log.cpp
        ${PROJECT_SOURCE_DIR}/log/log.h 
        ${PROJECT_SOURCE_DIR}/log/block_queue.h
        ${PROJECT_SOURCE_DIR}/buffer/buffer.cpp
        ${PROJECT_SOURCE_DIR}/buffer/buffer.h
        ${PROJECT_SOURCE_DIR}/timer/heaptimer.h
        ${PROJECT_SOURCE_DIR}/timer/heaptimer.cpp)


# add_subdirectory(./timer)

# add_subdirectory(./log)

# add_subdirectory(./pool)

# add_subdirectory(./http)

# add_subdirectory(./server)

add_executable(main-server main.cpp ${SRC})
target_link_libraries(main-server mysqlclient)
