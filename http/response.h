/**
 * @file response.h
 * @author balance (2570682750@qq.com)
 * @brief 响应类
 * @version 0.1
 * @date 2022-10-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <unordered_map>

#include "../log/log.h"
#include "buffer.h"

class Response {
 public:
  Response();
  ~Response();

  void init(const std::string &srcDir, std::string &path,
            bool isKeppAlive = false, int code = -1);
  void make_response(Buffer &buff);
  void unmap_file();
  char *file();
  size_t file_len() const;
  void error_content(Buffer &buff, std::string message);
  int get_code() const { return code_; }

 private:
  void add_state_line(Buffer &buff);
  void add_header(Buffer &buff);
  void add_content(Buffer &buff);

  void error_html();
  std::string get_file_type();

  int code_;
  bool isKeepAlive;

  std::string path_;
  std::string srcDir_;

  char *mmFile_;
  struct stat mmFileStat_;

  static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
  static const std::unordered_map<int, std::string> CODE_STATUS;
  static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif