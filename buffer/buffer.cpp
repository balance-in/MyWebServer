#include "buffer.h"

Buffer::Buffer(int buff_size) : buffer_(buff_size), read_pos(0), write_pos(0) {}

size_t Buffer::writable_bytes() const { return buffer_.size() - write_pos; }

size_t Buffer::readable_bytes() const { return write_pos - read_pos; }

size_t Buffer::prependable_bytes() const { return read_pos; }

const char *Buffer::peek() const { return begin_ptr() + read_pos; }

void Buffer::ensure_writable(size_t len) {
  if (writable_bytes() < len) {
    make_space(len);
  }
  assert(writable_bytes() >= len);
}

void Buffer::has_written(size_t len) { write_pos += len; }

void Buffer::retrieve(size_t len) {
  assert(len <= readable_bytes());
  read_pos += len;
}

void Buffer::retrieve_until(const char *end) { assert(peek() <= end); }

void Buffer::retrieve_all() {
  bzero(&buffer_[0], buffer_.size());
  read_pos = 0;
  write_pos = 0;
}

std::string Buffer::retrieve_all_tostr() {
  std::string str(peek(), readable_bytes());
  retrieve_all();
  return str;
}

const char *Buffer::begin_write_const() const {
  return begin_ptr() + write_pos;
}

char *Buffer::begin_write() { return begin_ptr() + write_pos; }

void Buffer::append(const std::string &str) {
  append(str.data(), str.length());
}
void Buffer::append(const char *str, size_t len) {
  assert(str);
  ensure_writable(len);
  std::copy(str, str + len, begin_write());
  has_written(len);
}
void Buffer::append(const void *data, size_t len) {
  assert(data);
  append(static_cast<const char *>(data), len);
}

void Buffer::append(const Buffer &buff) {
  append(buff.peek(), buff.readable_bytes());
}

ssize_t Buffer::readfd(int fd, int *Errno) {
  char buf[65536];
  struct iovec iov[2];
  const size_t writable = writable_bytes();

  //分散读，一次性读完
  iov[0].iov_base = begin_ptr() + write_pos;
  iov[0].iov_len = writable;
  iov[1].iov_base = buf;
  iov[1].iov_len = sizeof(buf);

  const ssize_t len = readv(fd, iov, 2);
  if (len < 0) {
    *Errno = errno;
  } else if (static_cast<size_t>(len) <= writable) {
    write_pos += len;
  } else {
    write_pos = buffer_.size();
    append(buf, len - writable);
  }
  return len;
}

ssize_t Buffer::writefd(int fd, int *Errno) {
  size_t read_size = readable_bytes();
  ssize_t len = write(fd, peek(), read_size);
  if (len < 0) {
    *Errno = errno;
    return len;
  }
  read_pos += len;
  return len;
}

char *Buffer::begin_ptr() { return &(*buffer_.begin()); }

const char *Buffer::begin_ptr() const { return &(*buffer_.begin()); }

void Buffer::make_space(size_t len) {
  if (writable_bytes() + prependable_bytes() < len) {
    buffer_.resize(write_pos + len + 1);
  } else {
    size_t readable = readable_bytes();
    std::copy(begin_ptr() + read_pos, begin_ptr() + write_pos, begin_ptr());
    read_pos = 0;
    write_pos = read_pos + readable;
    assert(readable == readable_bytes());
  }
}