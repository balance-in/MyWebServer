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
  fun();
  return 0;
}