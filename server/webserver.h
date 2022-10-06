/**
 * @file webserver.h
 * @author balance (2570682750@qq.com)
 * @brief 服务器类
 * @version 0.1
 * @date 2022-10-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>  // fcntl()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // close()

#include <unordered_map>

#include "../http/httpconn.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../timer/heaptimer.h"
#include "epoller.h"

class WebServer {
 public:
  WebServer(int port, int trig_mode, int timeout, bool opt_linger, int sql_port,
            const char *sql_uer, const char *sql_pwd, const char *db_name,
            int conn_pool_num, int thread_num, bool open_log, int log_level,
            int log_size);

  ~WebServer();
  void start();

 private:
  bool init_socket();
  void init_event_mode(int trig_mode);
  void add_client(int fd, sockaddr_in addr);

  void deal_listen();
  void deal_write(HttpConn *client);
  void deal_read(HttpConn *client);

  void send_error(int fd, const char *info);
  void extent_time(HttpConn *client);
  void close_conn(HttpConn *client);

  void on_read(HttpConn *client);
  void on_write(HttpConn *client);
  void on_process(HttpConn *client);

  static const int MAX_FD = 65536;

  static int set_non_blocking(int fd);

  int port_;
  bool open_linger_;
  int timeout_;
  bool is_close_;
  int listenfd_;
  char *srcDir_;

  uint32_t listen_event_;
  uint32_t conn_evnet_;

  std::unique_ptr<HeapTimer> timer_;
  std::unique_ptr<ThreadPool> thread_pool_;
  std::unique_ptr<Epoller> epoller_;
  std::unordered_map<int, HttpConn> users_;
};

#endif