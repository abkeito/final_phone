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

#define MAX_CLIENTS 5
#define BUFFER_SIZE 81920 // バッファサイズを増やす
//#define BUFFER_SIZE 163840 // バッファサイズを増やす

typedef struct
{
  int socket;
  pthread_t thread;
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
// 二次元配列とn
short data[MAX_CLIENTS][BUFFER_SIZE];
int ns[MAX_CLIENTS];

void broadcast_message(const char *message, int len, int sender_socket)
{
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < num_clients; i++)
  {
    if (clients[i].socket != sender_socket)
    {
      if (send(clients[i].socket, message, len, 0) == -1)
      {
        perror("send");
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void *my_push_back(short* buf, int s, int n){
  int client_i = 0;
  for(; client_i < num_clients; client_i++){
    if(clients[client_i].socket == s){
      break;
    }
  }
  int available_space = BUFFER_SIZE - ns[client_i];
  int copy_size = min(available_space, n / 2);
  //printf("Before memcpy: data[%d]=%p, client_i=%d, ns[%d]=%d\n", client_i, (void *)data[client_i], client_i, client_i, ns[client_i]);
  memcpy(data[client_i] + ns[client_i], buf, sizeof(short) * copy_size);
  ns[client_i] += copy_size;
  //printf("After memcpy:  data[%d]=%p, client_i=%d, ns[%d]=%d\n", client_i, (void *)data[client_i], client_i, client_i, ns[client_i]);
  pthread_mutex_unlock(&clients_mutex);
  return NULL;
}

void *handle_client(void *arg)
{
  int s = (int)(intptr_t)arg;
  short buf[BUFFER_SIZE];
  while (1)
  {
    // バイト数を指定してreadする。
    int n = recv(s, buf, (sizeof(short)*BUFFER_SIZE), 0);
    // int n = read(s, buf, sizeof(buf));
    //printf("n=%d\n", n);
    if (n == -1)
    {
      perror("recv");
      break;
    }
    if (n == 0)
    {
      break;
    }
    my_push_back(buf, s, n);
    // broadcast_message(buf, n, s);
  }
  //printf("client finished");
  close(s);


  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < num_clients; i++)
  {
    if (clients[i].socket == s)
    {
      clients[i] = clients[num_clients - 1];
      num_clients--;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
  return NULL;
}

/**
 * Dataをnsのmin_nまで足し算し、返り値にmin_nを返す。
 * @return min_n
 * @param sum_result 足し算結果を格納する配列
 */
int sum_data(int sum_result[])
{
  // min_nを求める
  int min_n = ns[0];
  for (int i = 0; i < num_clients; i++)
  {
    if (min_n > ns[i])
    {
      min_n = ns[i];
    }
  }

  // 足し算
  for (int i = 0; i < min_n; i++)
  {
    sum_result[i] = 0;
    for (int j = 0; j < num_clients; j++)
    {
      sum_result[i] += data[j][i];
    }
  }
  return min_n;
}

/**
 * sum_resultからdataの先頭からn個を引き、送信する。
 * @param n
 * @param sum_result
 */
void send_data(int n, int sum_result[])
{
  short send_buffer[n];
  for (int i = 0; i < num_clients; i++)
  {
    int sender_s = clients[i].socket;
    for (int j = 0; j < n; j++)
    {
      send_buffer[j] = sum_result[j] - data[i][j];
      // printf("%d - %d = ", sum_result[j],data[i][j]);
      // printf("%d\n", send_buffer[j]);
    }
    //voice change
    //pitchchange(sizeof(send_buffer), 12, send_buffer);
    //
    if (send(sender_s, send_buffer, sizeof(send_buffer), 0) == -1)
    {
      perror("send");
    }
  }
}

/**
 * dataのからnだけ先頭を削除する。
 */
void pop_front(int n)
{
  for (int i = 0; i < num_clients; i++)
  {
    for (int j = 0; j < ns[i] - n; j++)
    {
      data[i][j] = data[i][j + n];
    }
    ns[i] -= n;
  }
}

// pthreadに渡す関数
void *handle_sender()
{
  while (1)
  {
    if (num_clients < 2)
    {
      continue;
    }

    pthread_mutex_lock(&clients_mutex);
    int sum_result[BUFFER_SIZE];
    int min_n = sum_data(sum_result);
    send_data(min_n, sum_result);
    pop_front(min_n);
    pthread_mutex_unlock(&clients_mutex);
  }
}

void server_mode(int port)
{
  int ss = socket(PF_INET, SOCK_STREAM, 0);
  if (ss == -1)
  {
    die("socket");
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    die("bind");
  }
  if (listen(ss, 10) == -1)
  {
    die("listen");
  }

  log_message(LOG_FILE_SERVER, "[server] listening on %d", port);

  pthread_t sender_thread;
  pthread_create(&sender_thread, NULL, handle_sender, NULL);

  while (1)
  {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int s = accept(ss, (struct sockaddr *)&client_addr, &len);
    if (s == -1)
    {
      die("accept");
    }

    pthread_mutex_lock(&clients_mutex);
    if (num_clients >= MAX_CLIENTS)
    {
      close(s);
      pthread_mutex_unlock(&clients_mutex);
      continue;
    }
    clients[num_clients].socket = s;
    pthread_create(&clients[num_clients].thread, NULL, handle_client, (void *)(intptr_t)s);
    ns[num_clients] = 0;
    num_clients++;
    printf("client %d has joined\n", num_clients);
    pthread_mutex_unlock(&clients_mutex);

    log_message(LOG_FILE_SERVER, "[server] connected!!");
  }

  close(ss);
}
