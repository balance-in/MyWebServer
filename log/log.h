/**
 * @file Log.h
 * @author balance (2570682750@qq.com)
 * @brief 日志记录
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>

#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "block_queue.h"

class Log {
 private:
  char dir_name[128];    //路径名
  char log_name[128];    // log文件名
  int max_lines;         //日志最大行数
  int max_buf_log_size;  //日志缓冲区大小
  long long count_;      //日志行数
  int today_;            //当前那一天
  FILE *fp_;             //文件指针
  char *buf_;

  std::unique_ptr<block_queue<std::string>> log_queue;  //阻塞队列
  std::unique_ptr<std::thread> write_thread;            //写入线程
  std::mutex mutex_;

  bool is_async;  //是否同步
  bool is_open;   //是否关闭日志
  bool level_;    //日志等级

 private:
  Log();
  virtual ~Log();
  void async_write_log() {
    std::string log_str;
    while (log_queue->pop(log_str)) {
      std::lock_guard<std::mutex> locker(mutex_);
      fputs(log_str.c_str(), fp_);
    }
  }

 public:
  //懒汉模式，c++之后不用加锁
  static Log *get_instance() {
    static Log instance;
    return &instance;
  }

  bool IsOpen() { return is_open; }

  static void flush_log_thread() { Log::get_instance()->async_write_log(); }

  bool init(const char *file_name, int close_log, int buf_size = 8192,
            int split_lines = 5000000, int max_queue_size = 0);
  void write_log(int level, const char *format, ...);

  void flush(void);
};

#define LOG_BASE(level, format, ...)                \
  do {                                              \
    Log *log = Log::get_instance();                 \
    if (log->IsOpen()) {                            \
      log->write_log(level, format, ##__VA_ARGS__); \
      log->flush();                                 \
    }                                               \
  } while (0);

#define LOG_DEBUG(format, ...)         \
  do {                                 \
    LOG_BASE(0, format, ##__VA_ARGS__) \
  } while (0);
#define LOG_INFO(format, ...)          \
  do {                                 \
    LOG_BASE(1, format, ##__VA_ARGS__) \
  } while (0);
#define LOG_WARN(format, ...)          \
  do {                                 \
    LOG_BASE(2, format, ##__VA_ARGS__) \
  } while (0);
#define LOG_ERROR(format, ...)         \
  do {                                 \
    LOG_BASE(3, format, ##__VA_ARGS__) \
  } while (0);

#endif
