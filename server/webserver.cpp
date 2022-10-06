#include "webserver.h"

using namespace std;

WebServer::WebServer(int port, int trig_mode, int timeout, bool opt_linger,
                     int sql_port, const char *sql_uer, const char *sql_pwd,
                     const char *db_name, int conn_pool_num, int thread_num,
                     bool open_log, int log_level, int log_size)
    : port_(port),
      open_linger_(opt_linger),
      timeout_(timeout),
      is_close_(false),
      timer_(new HeapTimer()),
      thread_pool_(new ThreadPool(thread_num)),
      epoller_(new Epoller()) {
  srcDir_ = getcwd(nullptr, 256);
  assert(srcDir_);
  strncat(srcDir_, "/resources", 16);
  HttpConn::userCount = 0;
  HttpConn::srcDir = srcDir_;
  SqlConnPool::get_instance()->init("localhost", sql_port, sql_uer, sql_pwd,
                                    db_name, conn_pool_num);
  init_event_mode(trig_mode);
  if (!init_socket()) is_close_ = true;

  if (open_log) {
    Log::get_instance()->init("./log", true, log_size);
    if (is_close_) {
      LOG_ERROR("========== Server init error!==========");
    } else {
      LOG_INFO("========== Server init ==========");
      LOG_INFO("Port:%d, OpenLinger: %s", port_, opt_linger ? "true" : "false");
      LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
               (listen_event_ & EPOLLET ? "ET" : "LT"),
               (conn_evnet_ & EPOLLET ? "ET" : "LT"));
      LOG_INFO("LogSys level: %d", log_level);
      LOG_INFO("srcDir: %s", HttpConn::srcDir);
      LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", conn_pool_num,
               thread_num);
    }
  }
}

WebServer::~WebServer() {
  close(listenfd_);
  is_close_ = true;
  free(srcDir_);
  SqlConnPool::get_instance()->close_pool();
}

void WebServer::init_event_mode(int trig_mode) {
  listen_event_ = EPOLLRDHUP;
  conn_evnet_ = EPOLLONESHOT | EPOLLRDHUP;
  switch (trig_mode) {
    case 0:
      break;
    case 1:
      conn_evnet_ |= EPOLLET;
      break;
    case 2:
      listen_event_ |= EPOLLET;
      break;
    case 3:
      listen_event_ |= EPOLLET;
      conn_evnet_ |= EPOLLET;
      break;
    default:
      listen_event_ |= EPOLLET;
      conn_evnet_ |= EPOLLET;
      break;
  }
  HttpConn::is_et = (conn_evnet_ & EPOLLET);
}

void WebServer::start() {
  int time_ms = -1;
  if (!is_close_) {
    LOG_INFO("========== Server start ==========");
  }
  while (!is_close_) {
    if (timeout_ > 0) {
      time_ms = timer_->get_next_tick();
    }
    int eventCnt = epoller_->wait(time_ms);
    for (int i = 0; i < eventCnt; i++) {
      //处理事件
      int fd = epoller_->get_event_fd(i);
      uint32_t events = epoller_->get_events(i);
      if (fd == listenfd_) {
        deal_listen();
      } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        assert(users_.count(fd) > 0);
        close_conn(&users_[fd]);
      } else if (events & EPOLLIN) {
        assert(users_.count(fd) > 0);
        deal_read(&users_[fd]);
      } else if (events & EPOLLOUT) {
        assert(users_.count(fd) > 0);
        deal_write(&users_[fd]);
      } else {
        LOG_ERROR("Unexpected event");
      }
    }
  }
}

void WebServer::send_error(int fd, const char *info) {
  assert(fd > 0);
  int ret = send(fd, info, strlen(info), 0);
  if (ret < 0) {
    LOG_WARN("send error to client[%d] error!", fd);
  }
  close(fd);
}

void WebServer::close_conn(HttpConn *client) {
  assert(client);
  LOG_INFO("Client[%d] quit!", client->get_fd());
  epoller_->del_fd(client->get_fd());
  client->Close();
}

void WebServer::add_client(int fd, sockaddr_in addr) {
  assert(fd > 0);
  users_[fd].init(fd, addr);
  if (timeout_ > 0) {
    timer_->add(fd, timeout_,
                std::bind(&WebServer::close_conn, this, &users_[fd]));
  }
  epoller_->add_fd(fd, EPOLLIN | conn_evnet_);
  set_non_blocking(fd);
  LOG_INFO("Client[%d] in!", users_[fd].get_fd());
}

void WebServer::deal_listen() {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  do {
    int fd = accept(listenfd_, (struct sockaddr *)&addr, &len);
    if (fd <= 0) {
      return;
    } else if (HttpConn::userCount >= MAX_FD) {
      send_error(fd, "Server busy!");
      LOG_WARN("Client is full!");
      return;
    }
    add_client(fd, addr);
  } while (listen_event_ & EPOLLET);
}

void WebServer::deal_read(HttpConn *client) {
  assert(client);
  extent_time(client);
  thread_pool_->addworker(std::bind(&WebServer::on_read, this, client));
}

void WebServer::deal_write(HttpConn *client) {
  assert(client);
  extent_time(client);
  thread_pool_->addworker(std::bind(&WebServer::on_write, this, client));
}

void WebServer::extent_time(HttpConn *client) {
  assert(client);
  if (timeout_ > 0) {
    timer_->adjust(client->get_fd(), timeout_);
  }
}

void WebServer::on_read(HttpConn *client) {
  assert(client);
  int ret = -1;
  int read_errno = 0;
  ret = client->read(&read_errno);
  if (ret <= 0 && read_errno != EAGAIN) {
    close_conn(client);
    return;
  }
  on_process(client);
}

void WebServer::on_process(HttpConn *client) {
  if (client->process()) {
    epoller_->mod_fd(client->get_fd(), conn_evnet_ | EPOLLOUT);
  } else {
    epoller_->mod_fd(client->get_fd(), conn_evnet_ | EPOLLIN);
  }
}

void WebServer::on_write(HttpConn *client) {
  assert(client);
  int ret = -1;
  int write_errno = 0;
  ret = client->write(&write_errno);
  if (client->to_write_bytes() == 0) {
    //传输完成
    if (client->is_keep_alive()) {
      on_process(client);
      return;
    }
  } else if (ret < 0) {
    if (write_errno == EAGAIN) {
      //继续传输
      epoller_->mod_fd(client->get_fd(), conn_evnet_ | EPOLLOUT);
      return;
    }
  }
}

bool WebServer::init_socket() {
  int ret;
  struct sockaddr_in addr;
  if (port_ > 65535 || port_ < 1024) {
    LOG_ERROR("Port: %d error!", port_);
    return false;
  }
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port_);
  struct linger opt_linger = {0};
  if (open_linger_) {
    /* 优雅关闭: 直到所剩数据发送完毕或超时 */
    opt_linger.l_onoff = 1;
    opt_linger.l_linger = 1;
  }

  listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd_ < 0) {
    LOG_ERROR("Create socket error!");
    return false;
  }

  ret = setsockopt(listenfd_, SOL_SOCKET, SO_LINGER, &opt_linger,
                   sizeof(opt_linger));
  if (ret == -1) {
    LOG_ERROR("set socket setsockopt error !");
    close(listenfd_);
    return false;
  }

  ret = bind(listenfd_, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    LOG_ERROR("Bind Port: %d error!", port_);
    close(listenfd_);
    return false;
  }

  ret = listen(listenfd_, 6);
  if (ret < 0) {
    LOG_ERROR("Listen port: %d error!", port_);
    close(listenfd_);
    return false;
  }

  ret = epoller_->add_fd(listenfd_, listen_event_ | EPOLLIN);
  if (ret == 0) {
    LOG_ERROR("add listen error!");
    close(listenfd_);
    return false;
  }
  set_non_blocking(listenfd_);
  LOG_INFO("Server port: %d", port_);
  return true;
}

int WebServer::set_non_blocking(int fd) {
  assert(fd > 0);
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}