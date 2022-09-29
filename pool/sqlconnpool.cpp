/**
 * @file sqlconnpool.cpp
 * @author balance (2570682750@qq.com)
 * @brief 数据库连接池
 * @version 0.1
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "sqlconnpool.h"

SqlConnPool::SqlConnPool() {
  use_count = 0;
  free_count = 0;
}

void SqlConnPool::init(const char *host, int port, const char *user,
                       const char *pwd, const char *dbName, int connSize = 10) {
  assert(connSize > 0);
  for (int i = 0; i < connSize; i++) {
    MYSQL *sql = nullptr;
    sql = mysql_init(sql);
    if (!sql) {
      LOG_ERROR("MySql init error!");
      assert(sql);
    }
    sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
    if (!sql) {
      LOG_ERROR("Mysql connect error!");
    }
    conn_que.push(sql);
  }
  max_conn = connSize;
  sem_init(&semId_, 0, max_conn);
}

MYSQL *SqlConnPool::get_conn() {
  MYSQL *sql = nullptr;
  if (conn_que.empty()) {
    LOG_WARN("sql_conn_poll busy");
    return nullptr;
  }
  sem_wait(&semId_);
  {
    std::lock_guard<std::mutex> locker(mutex_);
    sql = conn_que.front();
    conn_que.pop();
  }
  return sql;
}

void SqlConnPool::free_conn(MYSQL *sql) {
  assert(sql);
  std::lock_guard<std::mutex> locker(mutex_);
  conn_que.push(sql);
  sem_post(&semId_);
}

void SqlConnPool::close_pool() {
  std::lock_guard<std::mutex> locker(mutex_);
  while (!conn_que.empty()) {
    auto item = conn_que.front();
    conn_que.pop();
    mysql_close(item);
  }
  mysql_library_end();
}

int SqlConnPool::get_free_conn_count() {
  std::lock_guard<std::mutex> locker(mutex_);
  return conn_que.size();
}

SqlConnPool::~SqlConnPool() { close_pool(); }