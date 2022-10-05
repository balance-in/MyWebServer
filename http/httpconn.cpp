#include "httpconn.h"

using namespace std;

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::is_et;

HttpConn::HttpConn() {
  fd_ = -1;
  addr_ = {0};
  isClose_ = true;
}

HttpConn::~HttpConn() { Close(); }

void HttpConn::init(int fd, const sockaddr_in& addr) {
  assert(fd > 0);
  userCount++;
  addr_ = addr;
  fd_ = fd;
  writeBuff_.retrieve_all();
  readBuffer_.retrieve_all();
  isClose_ = false;
  LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, get_ip(), get_port(),
           (int)userCount);
}

void HttpConn::Close() {
  response_.unmap_file();
  if (isClose_ == false) {
    isClose_ = true;
    userCount--;
    close(fd_);
    LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, get_ip(), get_port(),
             (int)userCount);
  }
}

int HttpConn::get_fd() const { return fd_; }

struct sockaddr_in HttpConn::get_addr() const {
  return addr_;
}

int HttpConn::get_port() const { return addr_.sin_port; }

ssize_t HttpConn::read(int* saveErrno) {
  ssize_t len = -1;
  do {
    len = readBuffer_.readfd(fd_, saveErrno);
    if (len <= 0) {
      break;
    }
  } while (is_et);
  return len;
}

ssize_t HttpConn::write(int* saveError) {
  ssize_t len = -1;
  do {
    len = writev(fd_, iov_, iovCnt_);
    if (len <= 0) {
      *saveError = errno;
      break;
    }
    if (iov_[0].iov_len + iov_[1].iov_len == 0) {
      break;
    } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
      iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
      iov_[1].iov_len -= (len - iov_[0].iov_len);
      if (iov_[0].iov_len) {
        writeBuff_.retrieve_all();
        iov_[0].iov_len = 0;
      }
    } else {
      iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
      iov_[0].iov_len -= len;
      writeBuff_.retrieve(len);
    }

  } while (is_et || to_write_bytes() > 10240);
  return len;
}

bool HttpConn::process() {
  request_.init();
  if (readBuffer_.readable_bytes() <= 0) {
    return false;
  } else if (request_.parse(readBuffer_)) {
    LOG_DEBUG("%s", request_.path().c_str());
    response_.init(srcDir, request_.path(), request_.is_keep_live(), 200);
  } else {
    response_.init(srcDir, request_.path(), false, 400);
  }

  response_.make_response(writeBuff_);
  // 响应头
  iov_[0].iov_base = const_cast<char*>(writeBuff_.peek());
  iov_[0].iov_len = writeBuff_.readable_bytes();
  iovCnt_ = 1;

  //文件
  if (response_.file_len() > 0 && response_.file()) {
    iov_[1].iov_base = response_.file();
    iov_[1].iov_len = response_.file_len();
    iovCnt_ = 2;
  }
  LOG_DEBUG("filesize:%d, %d to %d", response_.file_len(), iovCnt_,
            to_write_bytes());
  return true;
}