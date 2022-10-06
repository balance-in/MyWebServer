/**
 * @file epoller.h
 * @author balance (2570682750@qq.com)
 * @brief epoll封装
 * @version 0.1
 * @date 2022-10-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EPOLLER_H
#define EPOLLER_H

#include <assert.h>  // close()
#include <errno.h>
#include <fcntl.h>      // fcntl()
#include <sys/epoll.h>  //epoll_ctl()
#include <unistd.h>     // close()

#include <vector>

class Epoller {
 public:
  explicit Epoller(int max_event = 1024);

  ~Epoller();

  bool add_fd(int fd, uint32_t events);
  bool mod_fd(int fd, uint32_t events);
  bool del_fd(int fd);

  int wait(int timeout = -1);
  int get_event_fd(size_t i) const;
  uint32_t get_events(size_t i) const;

 private:
  int epollfd_;
  std::vector<struct epoll_event> events_;
};

#endif