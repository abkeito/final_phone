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

#define TEXT_SIZE 256
pthread_mutex_t npitch_mutex = PTHREAD_MUTEX_INITIALIZER; // npitchのロック
int npitch;

typedef struct {
    int s_audio;
    int s_text;
    FILE *listening_fd;
    FILE *speaking_fd;
} client_info_t;

void *sendtext_message(void *arg)
{ //

  client_info_t* client = (client_info_t*)arg; 
  if (!client) pthread_exit(NULL);  // ポインタのNULLチェック
  int s = client->s_text;
  char buf[TEXT_SIZE];
  int n;
  while ((n = read(0, buf, sizeof(buf))) > 0) {
      if (send(s, buf, n, 0) == -1) {
          perror("send");
          break;
      }
    //////voice change 判定 今の所segfault出る
      if (strncmp(buf, "voicechange", 11) == 0 && n > 12) { // コマンドの正確な比較と範囲チェック
          pthread_mutex_lock(&npitch_mutex);
          npitch = buf[12] - '0'; // '0' を引くことで数値に変換
          printf("voice changed -> %d\n", npitch);
          pthread_mutex_unlock(&npitch_mutex);
      }    
  }
    close(s);
    pthread_exit(NULL);
  return NULL;
}
//
void *receivetext_message(void *arg)
{ //
  client_info_t* client = (client_info_t*)arg; 
  if (!client) pthread_exit(NULL);  // ポインタのNULLチェック
  //
  int s = client->s_text;
  char buf[TEXT_SIZE];
  int recv_size;
  while ((recv_size = recv(s, buf, sizeof(buf), 0)) > 0) {
      if(write(1, buf, recv_size) == -1){
        perror("write");
        exit(1);
        }
      }
  close(s);  // ソケットのクローズ
  pthread_exit(NULL);  // スレッドの終了
}
//messages
void messages(client_info_t* client_info)
{ 
  pthread_t sendtext_thread;
  pthread_create(&sendtext_thread, NULL, sendtext_message, (void *)client_info);
  pthread_t receivetext_thread;
  pthread_create(&receivetext_thread, NULL, receivetext_message, (void *)client_info);
}
//
void *send_message(void *arg)
{ //
  client_info_t* client = (client_info_t*)arg; 
  //
  int s = client->s_audio;
  FILE* listening_fd = client->listening_fd;
  char buf[2048];
  //char buf[10];
  while (1)
  {
    int n = fread(buf,  1, sizeof(buf), listening_fd);
    if (n == -1)
    {
      die("read");
    }
    if (n == 0)
    {
      break;
    }
    /////
    if(npitch != 0){
      pitchchange(sizeof(buf), npitch, buf);
    }
    /////
    if (send(s, buf, n, 0) == -1)
    {
      die("send");
    }
  }
  return NULL;
}

void *receive_message(void *arg)
{ //
  client_info_t* client = (client_info_t*)arg; 
  //
  int s = client->s_audio;
  FILE* speaking_fd = client->speaking_fd;
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
    if (fwrite(buf, 1, n, speaking_fd) == -1)
    {
      die("write");
    }
  }
  return NULL;
}

void start_call(client_info_t* client_info)
{ 
  pthread_t send_thread;
  pthread_create(&send_thread, NULL, send_message, (void *)client_info);
  pthread_t receive_thread;
  pthread_create(&receive_thread, NULL, receive_message, (void *)client_info);
  while(1){
    sleep(1);
  }
}

void client_mode(int port, const char *ipaddr)
{ 
  int s_audio = socket(PF_INET, SOCK_STREAM, 0);
  int s_text  = socket(PF_INET, SOCK_STREAM, 0);
  if (s_audio  == -1 && s_text == -1)
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
  int ret = connect(s_audio, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1)
  {
    die("connect");
  }
  ret = connect(s_text, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1)
  {
    die("connect");
  }
  npitch = 0;
  /*つかちゃん*/
  FILE *listening_fd = popen("rec -r 44100 -b 16 -c 1 -e signed-integer -t raw - 2>/dev/null", "r");
  FILE *speaking_fd = popen("play -r 44100 -b 16 -c 1 -e signed-integer -t raw - 2>/dev/null", "w");

  if (listening_fd == NULL || speaking_fd == NULL) {
      perror("popen");
      close(s_audio); close(s_text);
      exit(1);
  }

  client_info_t *client_info = malloc(sizeof(client_info_t));
  client_info->s_text = s_text;
  client_info->s_audio = s_audio;
  client_info->listening_fd = listening_fd;
  client_info->speaking_fd = speaking_fd;
  /**/
  log_message(LOG_FILE_CLIENT, "[client] connected to %s:%d", ipaddr, port);
  
  start_call(client_info);
  messages(client_info);

  close(s_audio);
  close(s_text);
}
