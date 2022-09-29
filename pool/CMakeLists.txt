add_executable(threadpool threadpool.h test.cpp ${SRC} sqlconnpool.h sqlconnpool.cpp)
target_link_libraries(threadpool mysqlclient)
