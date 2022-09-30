#include "request.h"

using namespace std;

const unordered_set<string> Request::DEFAULT_HTML{
    "/index", "/regsiter", "/login", "welcome", "/video", "/picture"};

const unordered_map<string, int> Request::DEFAULT_HTML_TAG{
    {"/register.html", 0}, {"/login.html", 1}};

void Request::init() {
  method_ = path_ = version_ = body_ = "";
  state_ = REQUEST_LINE;
  header_.clear();
  post_.clear();
}

bool Request::is_keep_live() const {
  if (header_.count("Connection") == 1) {
    return header_.find("Connection")->second == "keep-alive" &&
           version_ == "1.1";
  }
  return false;
}

bool Request::parse(Buffer& buff) {
  const char CRLF[] = "\r\n";
  if (buff.readable_bytes() <= 0) return false;

  while (buff.readable_bytes() && state_ != FINISH) {
    const char* lineEnd =
        search(buff.peek(), buff.begin_write_const(), CRLF, CRLF + 2);
    std::string line(buff.peek(), lineEnd);
    switch (state_) {
      case REQUEST_LINE: {
        if (!parse_request_line(line)) return false;
        parse_path();
        break;
      }
      case HEADERS: {
        parse_header(line);
        if (buff.readable_bytes() <= 2) {
          state_ = FINISH;
        }
        break;
      }
      case BODY:
        parse_body(line);
        break;
      default:
        break;
    }
    if (lineEnd == buff.begin_write()) {
      break;
    }
    buff.retrieve_until(lineEnd + 2);
  }
  LOG_DEBUG("[%s],[%s],[%s]", method_.c_str(), path_.c_str(), version_.c_str());
  return true;
}

void Request::parse_path() {
  if (path_ == "/") {
    path_ = "/index.html";
  } else {
    for (auto& item : DEFAULT_HTML) {
      if (item == path_) {
        path_ += ".html";
        break;
      }
    }
  }
}

bool Request::parse_request_line(const string& line) {
  regex patten("^([^ ]*) ([^]*) HTTP/([^ ]*)$");
  smatch sub_match_;
  if (regex_match(line, sub_match_, patten)) {
    method_ = sub_match_[1];
    path_ = sub_match_[2];
    version_ = sub_match_[3];
    state_ = HEADERS;
    return true;
  }
  LOG_ERROR("Requestline error");
  return false;
}

void Request::parse_header(const string& line) {
  regex patten("^([^:]*): ?(.*)$");
  smatch sub_match_;
  if (regex_match(line, sub_match_, patten)) {
    header_[sub_match_[1]] = sub_match_[2];
  } else {
    state_ = BODY;
  }
}

void Request::parse_body(const string& line) {
  body_ = line;
  parse_post();
  state_ = FINISH;
  LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int Request::conver_hex(char ch) {
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  return ch;
}

void Request::parse_post() {
  if (method_ == "POST" &&
      header_["Content-Type"] == "application/x-www-form-urlencoded") {
    parse_from_url_encoded();
    if (DEFAULT_HTML_TAG.count(path_)) {
      int tag = DEFAULT_HTML_TAG.find(path_)->second;
      LOG_DEBUG("Tag:%d", tag);
      if (tag == 0 || tag == 1) {
        bool islogin = (tag == 1);
        if (user_verify(post_["username"], post_["password"], islogin)) {
          path_ = "/welcome.html";
        } else {
          path_ = "/error.html";
        }
      }
    }
  }
}

void Request::parse_from_url_encoded() {
  if (body_.size() == 0) return;

  string key, value;
  int num = 0;
  int n = body_.size();
  int i = 0, j = 0;

  for (; i < n; i++) {
    char ch = body_[i];
    switch (ch) {
      case '=':
        key = body_.substr(j, i - j);
        j = i + 1;
        break;
      case '+':
        body_[i] = ' ';
        break;
      case '%':
        num = conver_hex(body_[i + 1] * 16 + conver_hex(body_[i + 2]));
        body_[i + 2] = num % 10 + '0';
        body_[i + 1] = num / 10 + '0';
        break;
      case '&':
        value = body_.substr(j, i - j);
        j = i + 1;
    }
  }
}