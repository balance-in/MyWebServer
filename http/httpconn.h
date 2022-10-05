/**
 * @file httpconn.h
 * @author balance (2570682750@qq.com)
 * @brief http业务处理类
 * @version 0.1
 * @date 2022-10-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "request.h"
#include "response.h"

class HttpConn {
 public:
  HttpConn();
  ~HttpConn();

  void init(int sockfd, const sockaddr_in &addr);

  ssize_t read(int *saveErrno);
  ssize_t write(int *saveErrno);

  void Close();

  int get_fd() const;
  int get_port() const;
  const char *get_ip() const;
  sockaddr_in get_addr() const;

  bool process();

  int to_write_bytes() { return iov_[0].iov_len + iov_[1].iov_len; }

  bool is_keep_alive() const { return request_.is_keep_live(); }

  static bool is_et;
  static const char *srcDir;
  static std::atomic<int> userCount;

 private:
  int fd_;
  struct sockaddr_in addr_;

  bool isClose_;

  int iovCnt_;
  struct iovec iov_[2];

  Buffer readBuffer_;
  Buffer writeBuff_;

  Request request_;
  Response response_;
};

#endif