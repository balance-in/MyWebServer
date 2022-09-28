/**
 * @file log.cpp
 * @author balance (2570682750@qq.com)
 * @brief
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "log.h"

#include <string.h>

Log::Log() {
  count_ = 0;
  is_async = false;
  write_thread = nullptr;
  log_queue = nullptr;
  today_ = 0;
  fp_ = nullptr;
}

Log::~Log() {
  if (write_thread && write_thread->joinable()) {
    while (!log_queue->empty()) {
      log_queue->flush();
    }
    log_queue->Close();
    write_thread->join();
  }
  if (fp_) {
    std::lock_guard<std::mutex> locker(mutex_);
    flush();
    fclose(fp_);
  }
}
//异步需要设置阻塞队列的长度，同步不需要设置
bool Log::init(const char *file_name, int close_log, int buf_size = 8192,
               int split_lines = 5000000, int max_queue_size = 0) {
  if (max_queue_size >= 1) {
    is_async = true;
    if (!log_queue) {
      std::unique_ptr<block_queue<std::string>> new_queue(
          new block_queue<std::string>(max_buf_log_size));
      log_queue = std::move(new_queue);

      std::unique_ptr<std::thread> New_thread(
          new std::thread(flush_log_thread));
      write_thread = std::move(New_thread);
    }
  } else {
    is_async = false;
  }

  is_close_log = close_log;
  max_buf_log_size = buf_size;
  buf_ = new char[max_buf_log_size];
  memset(buf_, '\0', max_buf_log_size);
  max_lines = split_lines;

  time_t t = time(NULL);
  struct tm *sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;

  const char *p = strrchr(file_name, '/');
  char log_full_name[256] = {0};

  if (p == NULL) {
    snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900,
             my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
  } else {
    strcpy(log_name, p + 1);
    strncpy(dir_name, file_name, p - file_name + 1);
    snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name,
             my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
  }

  today_ = my_tm.tm_mday;

  {
    std::lock_guard<std::mutex> locker(mutex_);
    if (fp_) {
      flush();
      fclose(fp_);
    }
  }

  fp_ = fopen(log_full_name, "a");
  if (fp_ == nullptr) {
    return false;
  }
  return true;
}

void Log::write_log(int level, const char *format, ...) {
  struct timeval now = {0, 0};
  gettimeofday(&now, nullptr);
  time_t tSec = now.tv_sec;
  struct tm *sysTime = localtime(&tSec);
  struct tm my_tm = *sysTime;
  char s[16] = {0};

  switch (level) {
    case 0:
      strcpy(s, "[debug]:");
      break;
    case 1:
      strcpy(s, "[info]:");
      break;
    case 2:
      strcpy(s, "[warn]:");
    case 3:
      strcpy(s, "[errno]:");
    default:
      strcpy(s, "[info]:");
      break;
  }

  //写入一个log
  std::unique_lock<std::mutex> locker(mutex_);
  count_++;
  //若日期改变或超过最大行，重新生成log文件
  if (today_ != my_tm.tm_mday || (count_ && (count_ % max_lines == 0))) {
    char new_log[256] = {0};
    flush();
    fclose(fp_);
    char tail[16] = {0};

    snprintf(tail, 16, "%d_%02d_%20d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1,
             my_tm.tm_mday);

    if (today_ != my_tm.tm_mday) {
      snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
      today_ = my_tm.tm_mday;
      count_ = 0;
    } else {
      snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name,
               count_ / max_lines);
    }
    fp_ = fopen(new_log, "a");
    assert(fp_ != nullptr);
  }

  locker.unlock();

  va_list valist;
  va_start(valist, format);

  std::string log_str;
  locker.lock();

  int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                   my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                   my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

  int m = vsnprintf(buf_ + n, max_buf_log_size - 1, format, valist);
  buf_[n + m] = '\n';
  buf_[n + m + 1] = '\0';
  log_str = buf_;

  locker.unlock();
  if (is_async && !log_queue->full()) {
    log_queue->push_back(log_str);
  } else {
    locker.lock();
    fputs(log_str.c_str(), fp_);
    locker.unlock();
  }

  va_end(valist);
}

void Log::flush() {
  if (is_async) {
    log_queue->flush();
  }
  fflush(fp_);
}