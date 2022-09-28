/**
 * @file test.cpp
 * @author balance (2570682750@qq.com)
 * @brief test threadpool
 * @version 0.1
 * @date 2022-09-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <iostream>

#include "threadpool.h"

int main() {
  auto lam = [](int i, int j) { std::cout << i + j; };
  std::function<void()> fun(std::bind(lam, 1, 2));
  ThreadPool *pool = new ThreadPool(3);
  pool->addworker([](int i, int j) { std::cout << 2 + 4 << std::endl; }, 2, 4);
  // fun();
  return 0;
}