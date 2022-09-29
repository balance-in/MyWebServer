#include "log.h"

int main() {
  Log::get_instance()->init("log.txt", true);
  LOG_DEBUG("ssss");
  LOG_INFO("SSS");
  LOG_WARN("ssaa");
  LOG_ERROR("sxcx000");
}