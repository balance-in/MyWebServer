#include <string.h>
#include <unistd.h>

#include "log.h"

int main() {
  Log::get_instance()->init("log.txt", true);
  LOG_DEBUG("ssss");
  LOG_INFO("SSS");
  LOG_WARN("ssaa");
  LOG_ERROR("sxcx000");
  char *path;
  path = getcwd(nullptr, 256);
  strncat(path, "/resources", 16);
  printf("%s", path);
  return 0;
}