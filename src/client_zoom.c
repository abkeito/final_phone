#include "network.h"
#include "helper.h"
#include "fft.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

void *send_message(void *arg)
{ int npitch = 3;
  int s = (int)(intptr_t)arg;
  char buf[2048];
  while (1)
  {
    int n = read(0, buf, sizeof(buf));
    if (n == -1)
    {
      die("read");
    }
    if (n == 0)
    {
      break;
    }
    /////
    pitchchange(sizeof(buf), npitch, buf);
    /////
    if (send(s, buf, n, 0) == -1)
    {
      die("send");
    }
  }
  return NULL;
}

void *receive_message(void *arg)
{
  int s = (int)(intptr_t)arg;
  char buf[1000];
  while (1)
  {
    int n = recv(s, buf, sizeof(buf), 0);
    if (n == -1)
    {
      die("recv");
    }
    if (n == 0)
    {
      break;
    }
    if (write(1, buf, n) == -1)
    {
      die("write");
    }
  }
  return NULL;
}

void start_call(int s)
{
  pthread_t send_thread;
  pthread_create(&send_thread, NULL, send_message, (void *)(intptr_t)s);

  receive_message((void *)(intptr_t)s);
}

void client_mode(int port, const char *ipaddr)
{
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1)
  {
    die("socket");
  }
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_aton(ipaddr, &addr.sin_addr) == 0)
  {
    die("inet_aton");
  }
  int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1)
  {
    die("connect");
  }

  log_message(LOG_FILE_CLIENT, "[client] connected to %s:%d", ipaddr, port);

  start_call(s);
  close(s);
}
