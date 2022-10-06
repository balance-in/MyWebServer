/**
 * @file heaptimer.h
 * @author balance (2570682750@qq.com)
 * @brief 时间堆
 * @version 0.1
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <arpa/inet.h>
#include <assert.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_map>

#include "../log/log.h"

typedef std::function<void()> TimerCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds Ms;
typedef Clock::time_point TimeStamp;

struct TimerNode {
  int id;
  TimeStamp expires;
  TimerCallBack time_cb;
  bool operator<(const TimerNode &t) { return expires < t.expires; }
};

class HeapTimer {
 public:
  HeapTimer() { heap_.reserve(64); }
  ~HeapTimer() { clear(); }

  void adjust(int id, int newExpires);

  void add(int id, int timeout, const TimerCallBack &cb);

  void do_work(int id);

  void clear();

  void tick();

  void pop();

  int get_next_tick();

 private:
  void del_timer(size_t i);
  void sift_up(size_t i);
  bool sift_down(size_t index, size_t n);
  void swap_node(size_t i, size_t j);

  std::vector<TimerNode> heap_;

  std::unordered_map<int, size_t> ref_;
};

#endif