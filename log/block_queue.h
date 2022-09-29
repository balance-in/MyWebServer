/**
 * @file block_queue.h
 * @author balance (2570682750@qq.com)
 * @brief 循环阻塞队列
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
#include <assert.h>
#include <sys/time.h>

#include <condition_variable>
#include <deque>
#include <mutex>

template <class T>
class block_queue {
 private:
  std::mutex mutex_;
  std::condition_variable cond_producer;
  std::condition_variable cond_consumer;

  std::deque<T> deque_;

  size_t capacity_;
  bool isClose_;

 public:
  explicit block_queue(size_t MaxCapacity = 1000);
  ~block_queue();
  void clear();
  bool empty();
  bool full();
  void Close();
  size_t size();
  size_t capacity();
  T front();
  T back();
  void push_back(const T &item);
  void push_front(const T &item);
  bool pop(T &item);
  bool pop(T &item, int timeout);
  void flush();
};

template <class T>
block_queue<T>::block_queue(size_t MaxCapacity) : capacity_(MaxCapacity) {
  assert(MaxCapacity > 0);
  isClose_ = false;
}

template <class T>
block_queue<T>::~block_queue() {
  Close();
}

template <class T>
void block_queue<T>::Close() {
  {
    std::lock_guard<std::mutex> locker(mutex_);
    deque_.clear();
    isClose_ = true;
  }
  cond_producer.notify_all();
  cond_consumer.notify_all();
}

template <class T>
void block_queue<T>::flush() {
  cond_consumer.notify_one();
}

template <class T>
void block_queue<T>::clear() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.clear();
}

template <class T>
T block_queue<T>::front() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.front();
}

template <class T>
T block_queue<T>::back() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.back();
}

template <class T>
size_t block_queue<T>::size() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.size();
}

template <class T>
size_t block_queue<T>::capacity() {
  std::lock_guard<std::mutex> locker(mutex_);
  return capacity_;
}

template <class T>
void block_queue<T>::push_back(const T &item) {
  std::unique_lock<std::mutex> locker(mutex_);
  while (deque_.size() >= capacity_) {
    cond_producer.wait(locker);
  }
  deque_.push_back(item);
  cond_consumer.notify_one();
}

template <class T>
void block_queue<T>::push_front(const T &item) {
  std::unique_lock<std::mutex> locker(mutex_);
  while (deque_.size() >= capacity_) {
    cond_producer.wait(locker);
  }
  deque_.push_front(item);
  cond_consumer.notify_one();
}

template <class T>
bool block_queue<T>::empty() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.empty();
}

template <class T>
bool block_queue<T>::full() {
  std::lock_guard<std::mutex> locker(mutex_);
  return deque_.size() >= capacity_;
}

template <class T>
bool block_queue<T>::pop(T &item) {
  std::unique_lock<std::mutex> locker(mutex_);
  while (deque_.empty()) {
    cond_consumer.wait(locker);
    if (isClose_) {
      return false;
    }
  }
  item = deque_.front();
  deque_.pop_front();
  cond_producer.notify_one();
  return true;
}

template <class T>
bool block_queue<T>::pop(T &item, int timeout) {
  std::unique_lock<std::mutex> locker(mutex_);
  while (deque_.empty()) {
    if (cond_consumer.wait_for(locker, std::chrono::seconds(timeout)) ==
        std::cv_status::timeout) {
      return false;
    }
    if (isClose_) {
      return false;
    }
  }
  item = deque_.front();
  deque_.pop_front();
  cond_producer.notify_one();
  return true;
}

#endif