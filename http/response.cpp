#include "response.h"

using namespace std;

const unordered_map<string, string> Response::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const unordered_map<int, string> Response::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const unordered_map<int, string> Response::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

Response::Response() {
  code_ = -1;
  path_ = srcDir_ = "";
  isKeepAlive_ = false;
  mmFile_ = nullptr;
  mmFileStat_ = {0};
}

Response::~Response() { unmap_file(); }

void Response::init(const std::string &srcDir, std::string &path,
                    bool isKeppAlive, int code) {
  assert(srcDir != "");
  if (mmFile_) {
    unmap_file();
  }
  code_ = code;
  isKeepAlive_ = isKeppAlive;
  path_ = path;
  srcDir_ = srcDir;
  mmFile_ = nullptr;
  mmFileStat_ = {0};
}

void Response::make_response(Buffer &buff) {
  //判断请求的资源文件
  if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 ||
      S_ISDIR(mmFileStat_.st_mode)) {
    code_ = 404;
  } else if (!(mmFileStat_.st_mode & S_IROTH)) {
    code_ = 403;
  } else if (code_ == -1) {
    code_ = 200;
  }
  error_html();
  add_state_line(buff);
  add_header(buff);
  add_content(buff);
}

char *Response::file() { return mmFile_; }

size_t Response::file_len() const { return mmFileStat_.st_size; }

void Response::error_html() {
  if (CODE_PATH.count(code_) == 1) {
    path_ = CODE_PATH.find(code_)->second;
    stat((srcDir_ + path_).data(), &mmFileStat_);
  }
}

void Response::add_state_line(Buffer &buff) {
  string status;
  if (CODE_STATUS.count(code_) == 1) {
    status = CODE_STATUS.find(code_)->second;
  } else {
    code_ = 400;
    status = CODE_STATUS.find(400)->second;
  }
  buff.append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

void Response::add_header(Buffer &buff) {
  buff.append("Connection: ");
  if (isKeepAlive_) {
    buff.append("keep-alive\r\n");
    buff.append("keep-alive: max=6, timeout=120\r\n");
  } else {
    buff.append("close\r\n");
  }
  buff.append("Content-type: " + get_file_type() + "\r\n");
}

void Response::add_content(Buffer &buff) {
  int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
  if (srcFd < 0) {
    error_content(buff, "File Not Found");
    return;
  }
  /* 将文件映射到内存提高文件的访问速度
  MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
  LOG_DEBUG("file path %s", (srcDir_ + path_).data());
  int *mmRet =
      (int *)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
  if (*mmRet == -1) {
    error_content(buff, "File NOT Found!");
    return;
  }
  mmFile_ = (char *)mmRet;
  close(srcFd);
  buff.append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void Response::unmap_file() {
  if (mmFile_) {
    munmap(mmFile_, mmFileStat_.st_size);
    mmFile_ = nullptr;
  }
}

string Response::get_file_type() {
  //判断文件类型
  string::size_type idx = path_.find_last_of('.');
  if (idx == string::npos) {
    return "text/plain";
  }
  string suffix = path_.substr(idx);
  if (SUFFIX_TYPE.count(suffix) == 1) {
    return SUFFIX_TYPE.find(suffix)->second;
  }
  return "text/palin";
}

void Response::error_content(Buffer &buff, string message) {
  string body;
  string status;
  body += "<html><title>Error</title>";
  body += "<body bgcolor=\"ffffff\">";
  if (CODE_STATUS.count(code_) == 1) {
    status = CODE_STATUS.find(code_)->second;
  } else {
    status = "Bad Requset";
  }
  body += to_string(code_) + " : " + status + "\n";
  body += "<p>" + message + "</p>";
  body += "<hr><em>MyWebServer</em></body></html>";

  buff.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
  buff.append(body);
}
