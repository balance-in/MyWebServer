#include "buffer/buffer.h"
#include "http/httpconn.h"
#include "log/log.h"
#include "pool/sqlconnpool.h"
#include "pool/threadpool.h"
#include "server/webserver.h"
#include "timer/heaptimer.h"

int main() {
  WebServer server(1316, 3, 60000, false, 3306, "root", "root", "serverdb", 12,
                   6, true, 1, 1024);
  server.start();
  return 0;
}