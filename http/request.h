/**
 * @file request.h
 * @author balance (2570682750@qq.com)
 * @brief 请求解析类
 * @version 0.1
 * @date 2022-09-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <errno.h>
#include <mysql/mysql.h>

#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"

class Request {
 public:
  enum PARSE_STATE {
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH,
  };

  enum HTTP_CODE {
    NO_REQUEST = 0,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURSE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION,
  };

  Request() { init(); }
  ~Request() = default;

  void init();
  bool parse(Buffer &buff);

  std::string path() const;
  std::string &path();
  std::string method() const;
  std::string version() const;
  std::string get_post(const std::string &key) const;
  std::string get_post(const char *key) const;

  bool is_keep_live() const;

  /*
  todo
  void HttpConn::ParseFormData() {}
  void HttpConn::ParseJson() {}
  */

 private:
  bool parse_request_line(const std::string &line);
  void parse_header(const std::string &line);
  void parse_body(const std::string &line);

  void parse_path();
  void parse_post();
  void parse_from_url_encoded();

  static bool user_verify(const std::string &name, const std::string &pwd,
                          bool is_login);

  PARSE_STATE state_;
  std::string method_, path_, version_, body_;
  std::unordered_map<std::string, std::string> header_;
  std::unordered_map<std::string, std::string> post_;

  static const std::unordered_set<std::string> DEFAULT_HTML;
  static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
  static int conver_hex(char ch);
};
#endif