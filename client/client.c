#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<pthread.h>
#include "common.h"

#define MAX_LINE 100
#define SERV_PORT 8000
#define POKER_MSG_LEN 14


#define handle_error(msg) \
  do { \
     perror(msg);\
     exit(EXIT_FAILURE);\
   }while(0)

void stop(int signo);
void *handleRecv(void *arg);
void *handleWrite(void *arg);
void handleMsg(const char *buf);
int getSendMsg(char *buf, int len, int action, int clientId);

int sockfd;
int action = POKER_ACTION_LOGIN;
int clientId = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char g_clientSN[CLIENT_SN_LEN] = {0};

int main(int argc,char *argv[])
{
  struct sockaddr_in server;
  //char buf[MAX_LINE];
  //int n;
  pthread_t pid_r = -1;
  pthread_t pid_w = -1;
  #ifdef AUTO_CLIENT
    if (argc < 2) {
        printf("ERROR:argc < 2\n");
        return -1;
    }
    clientId = atoi(argv[1]);
    printf("clientId = %d\n", clientId);
  #endif
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  signal(SIGINT,stop);
  memset(&server,0,sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(SERV_PORT);
  if(-1 == connect(sockfd,(struct sockaddr *)&server,sizeof(server)))
  {
    handle_error("connet");
  }

#ifdef AUTO_CLIENT
  //pthread_mutex_lock(&mutex);
  char loginMsg[16] = {0};
  //snprintf(loginMsg, sizeof(loginMsg), "%4d%4d\n", action, clientId);
  getSendMsg(loginMsg, POKER_MSG_LEN, action, clientId);
  write(sockfd, loginMsg, strlen(loginMsg));
  action = POKER_ACTION_PRIV;
  //pthread_mutex_unlock(&mutex);
#endif

  if(pthread_create(&pid_r,NULL,handleRecv,NULL) < -1)
  {
     handle_error("pthread_create recv failed..\n");
  }
  if(pthread_create(&pid_w,NULL,handleWrite,NULL) < -1)
  {
     handle_error("pthread_create write failed..\n");
  }
  for(;;);
  printf("finished ....\n");
  close(sockfd);
  return 0;
}

void stop(int signo)
{
  printf("stop...\n");
  close(sockfd);
  exit(0);
}

void *handleWrite(void *arg)
{
  char buf[MAX_LINE] = {0};
  while(1) {
    pthread_mutex_lock(&mutex);
    memset(buf,0,MAX_LINE);
 #ifdef AUTO_CLIENT
    pthread_cond_wait(&cond, &mutex);
    printf("befor getSendMsg...\n");
    getSendMsg(buf, POKER_MSG_LEN, action, clientId);
#else
    fgets(buf,MAX_LINE,stdin);
    if(!strncmp(buf,"closed",strlen("closed"))){
       break;
    }
#endif
    pthread_mutex_unlock(&mutex);
    write(sockfd,buf,strlen(buf));
    printf("send:%s",buf);
  }

}

void *handleRecv(void *arg)
{
  char buf[MAX_LINE] = {0};
  int num;
  for(;;)
  {
    num = read(sockfd,buf,MAX_LINE);
    if(num  == -1 || num == 0)
    {
      handle_error("read error..\n");
      exit(0); 
    }
    else
    {
      printf("recv:%s\n",buf);
      handleMsg(buf);
      //fflush(stdout);
      if(strncmp(buf,"server:close",MAX_LINE) == 0)
      {
        exit(0);
      }
    }
    memset(buf,0,MAX_LINE); 
  }

}

void handleMsg(const char *buf)
{
    int len = strlen(buf);
    int i = 0;
    int command = 0;
    if(len < sizeof(int) || len > 64)
    {
    printf("buf len < 4 or > 64, len is %d\n",len);
    return;
    }
    char *temp = (char *)malloc(len);
    if(temp == NULL)
    {
    printf("malloc failed.\n");
    return;
    }
    memset(temp,0,sizeof(temp));
    strncpy(temp,buf+CLIENT_SN_LEN,sizeof(int));
    //pthread_mutex_lock(&mutex);
    command = atoi(temp);
    printf("command: %d\n",command);
    action = (command > action)?command:action;
    if(action == POKER_ACTION_PRIV) {
     //dumpPrivMsg(temp);
     action = POKER_ACTION_BET;
    }
#ifdef AUTO_CLIENT
    if (strstr(buf, "please bet for")) {
    printf("pthread_cond_signal start...\n");
    pthread_cond_signal(&cond);
    }
#endif
    // pthread_mutex_unlock(&mutex);
}

int getSendMsg(char *buf, int len, int action, int clientId)
{
    printf("[%s] start, action=%d, clientId=%d\n", __FUNCTION__, action ,clientId);
    if (buf == NULL || len < POKER_MSG_LEN) {
        return -1;
    }
    snprintf(buf, POKER_MSG_LEN+CLIENT_SN_LEN, "%4d%4d0004%4d\n", clientId, action, action*100+clientId*10);
    printf("[%s] buf=%s\n", __FUNCTION__, buf);
    return 0;
}
