/**
 * @file sqlconnpool.h
 * @author balance (2570682750@qq.com)
 * @brief 数据库连接池
 * @version 0.1
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <semaphore.h>

#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "../log/log.h"

class SqlConnPool {
 public:
  static SqlConnPool *get_instance() {
    static SqlConnPool instance;
    return &instance;
  }

  MYSQL *get_conn();
  void free_conn(MYSQL *conn);
  int get_free_conn_count();

  void init(const char *host, int port, const char *user, const char *pwd,
            const char *dbName, int connSize);
  void close_pool();

 private:
  SqlConnPool();
  ~SqlConnPool();

  int max_conn;
  int use_count;
  int free_count;

  std::queue<MYSQL *> conn_que;
  std::mutex mutex_;
  sem_t semId_;
};

class SqlConnRALL {
 public:
  SqlConnRALL(MYSQL **sql, SqlConnPool *conn_pool) {
    assert(conn_pool);
    *sql = conn_pool->get_conn();
    sql_ = *sql;
    conn_pool_ = conn_pool;
  }

  ~SqlConnRALL() {
    if (sql_) conn_pool_->free_conn(sql_);
  }

 private:
  MYSQL *sql_;
  SqlConnPool *conn_pool_;
};

#endif