/**
 * @file buffer.h
 * @author balance (2570682750@qq.com)
 * @brief 封装缓冲区
 * @version 0.1
 * @date 2022-09-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef BUFFER_H
#define BUFFER_H
#include <assert.h>
#include <sys/uio.h>  //readv
#include <unistd.h>   // write

#include <atomic>
#include <cstring>  //perror
#include <iostream>
#include <vector>  //readv

class Buffer {
 public:
  Buffer(int buff_size = 1024);
  ~Buffer() = default;

  size_t writable_bytes() const;
  size_t readable_bytes() const;
  size_t prependable_bytes() const;

  const char *peek() const;
  void ensure_writable(size_t len);
  void has_written(size_t len);

  void retrieve(size_t len);
  void retrieve_until(const char *end);

  void retrieve_all();
  std::string retrieve_all_tostr();

  const char *begin_write_const() const;
  char *begin_write();

  void append(const std::string &str);
  void append(const char *str, size_t len);
  void append(const void *data, size_t len);
  void append(const Buffer &buff);

  ssize_t readfd(int fd, int *Errno);
  ssize_t writefd(int fd, int *Errno);

 private:
  char *begin_ptr();
  const char *begin_ptr() const;
  void make_space(size_t len);

  std::vector<char> buffer_;
  std::atomic<std::size_t> read_pos;
  std::atomic<std::size_t> write_pos;
};

#endif