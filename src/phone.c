#include "helper.h"
#include "network.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc == 3)
  {
    int port = atoi(argv[2]);
    client_mode(port, argv[1]);
  }
  else if (argc == 2)
  {
    int port = atoi(argv[1]);
    server_mode(port);
  }
  else
  {
    die("usage: ./a.out <port> or ./a.out <ipaddr> <port>");
  }
  return 0;
}
