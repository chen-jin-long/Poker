#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<pthread.h>
#include <sys/time.h>
#include "common.h"
#include "msg_json.h"

#define TMP_BUF_LEN 256
#define SERV_PORT 8000
#define POKER_MSG_LEN 14
#define INIT_BET_MONEY 100
#define CLIENT_HEARTBEAT_TIME 5
#define CLIENT_START_LOOP_TIME 2
#define CLIENT_STOP_LOOP_TIME 0

#define handle_error(msg) \
  do { \
     perror(msg);\
     exit(EXIT_FAILURE);\
   }while(0)

void stop(int signo);
void signalHandler(int signo);
void *handleRecv(void *arg);
void *handleWrite(void *arg);
void handleJsonMsg(QueueMsg *queue_msg);
//void handleMsg(const char *buf);
//int getSendMsg(char *buf, int len, int action, int clientId);

//int sockfd;
//int action = POKER_ACTION_LOGIN;
//int clientId = 0;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//char g_clientSN[CLIENT_SN_LEN] = {0};

Client * g_clt = NULL;
struct itimerval new_value, old_value;

void setLoopTime(unsigned int loopTime)
{
    new_value.it_value.tv_sec = loopTime;
    new_value.it_value.tv_usec = 0;
    new_value.it_interval.tv_sec = loopTime;
    new_value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &new_value, &old_value);
}

int main(int argc,char *argv[])
{
  struct sockaddr_in server;
  //char buf[TMP_BUF_LEN];
  //int n;
  pthread_t pid_r = -1;
  pthread_t pid_w = -1;
  #ifdef AUTO_CLIENT
    if (argc < 2) {
        printf("ERROR:argc < 2\n");
        return -1;
    }
    int clientId = atoi(argv[1]);
    printf("clientId = %d\n", clientId);
  #endif
  g_clt = (Client *)malloc(sizeof(Client));
  if (g_clt) {
    memset(g_clt, 0, sizeof(Client));
    PokerMsgBuf *msgBuf = (PokerMsgBuf *)malloc(sizeof(PokerMsgBuf));
    if (msgBuf) {
      memset(msgBuf, 0, sizeof(PokerMsgBuf));
      g_clt->msgBuf = msgBuf;
    } else {
        free(g_clt);
        g_clt = NULL;
        return -1;
    }
  } else {
    return -1;
  }
  g_clt->clientId = clientId;
  g_clt->betMoney = (g_clt->clientId + 1) * INIT_BET_MONEY;
  //g_clt->mutex = PTHREAD_MUTEX_INITIALIZER;
  //g_clt->cond = PTHREAD_COND_INITIALIZER;
  pthread_mutex_init(&g_clt->mutex, NULL);
  pthread_cond_init(&g_clt->cond, NULL);
  g_clt->sockFd = socket(AF_INET,SOCK_STREAM,0);
  g_clt->msgBuf->connId = g_clt->sockFd;
  signal(SIGINT, signalHandler);
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(SERV_PORT);
  if(-1 == connect(g_clt->sockFd,(struct sockaddr *)&server,sizeof(server)))
  {
    handle_error("connet");
  }

  strncpy(g_clt->action, "login", sizeof(g_clt->action) - 1);

#ifdef AUTO_CLIENT
  //pthread_mutex_lock(&g_clt->mutex);
  //char loginMsg[16] = {0};
  //snprintf(loginMsg, sizeof(loginMsg), "%4d%4d\n", action, clientId);
  //getSendMsg(loginMsg, POKER_MSG_LEN, action, clientId);
  char *loginMsg = build_poker_msg(g_clt->clientId, g_clt->action, "login_2021-01-8 14:18:00");
  if (loginMsg) {
    write(g_clt->sockFd, loginMsg, strlen(loginMsg));
    free(loginMsg);
    loginMsg = NULL;
  }

  //action = POKER_ACTION_PRIV;
  //pthread_mutex_unlock(&g_clt->mutex);
#endif

  if(pthread_create(&pid_r,NULL,handleRecv,NULL) < -1)
  {
     handle_error("pthread_create recv failed..\n");
  }
  if(pthread_create(&pid_w,NULL,handleWrite,NULL) < -1)
  {
     handle_error("pthread_create write failed..\n");
  }

  signal(SIGALRM, signalHandler);

  setLoopTime(CLIENT_START_LOOP_TIME);

  for(;;);
  printf("finished ....\n");
  stop(0);
  return 0;
}

void stop(int signo)
{
  printf("stop...\n");
  if (g_clt) {
    close(g_clt->sockFd);
    if (g_clt->msgBuf) {
      free(g_clt->msgBuf);
      g_clt->msgBuf = NULL;
    }
    free(g_clt);
    g_clt = NULL;
  }
  exit(0);
}

void signalHandler(int signo)
{
    time_t now = time(NULL);
    switch (signo){
        case SIGALRM:
            printf("Caught the SIGALRM signal!\n");
            pthread_mutex_lock(&g_clt->mutex);
            if (now - g_clt->lastMsgTime >= CLIENT_HEARTBEAT_TIME) {
                g_clt->needHeartBeat = 1;
            }
            pthread_cond_signal(&g_clt->cond);
            pthread_mutex_unlock(&g_clt->mutex);
            printf("[%s] SIGALRM done.\n", __FUNCTION__);
            break;
        case SIGINT:
          stop(signo);
          break;
   }
}

void *handleWrite(void *arg)
{
  char buf[TMP_BUF_LEN] = {0};
  char *betMsg = NULL;
  int len = 0;
  time_t time_stamp = 0;
  while(1) {
      pthread_mutex_lock(&g_clt->mutex);
      memset(buf,0,TMP_BUF_LEN);
    #ifdef AUTO_CLIENT
      printf("before cond wait...\n");
      // 防止虚假唤醒
      while(g_clt->needBet == 0 && g_clt->needHeartBeat == 0) {
          pthread_cond_wait(&g_clt->cond, &g_clt->mutex);
      }
      printf("getSendMsg...\n");
      //getSendMsg(buf, POKER_MSG_LEN, action, g_clt->clientId);
      snprintf(buf, sizeof(buf), "%d", g_clt->betMoney + INIT_BET_MONEY);
    #else
      fgets(buf,TMP_BUF_LEN,stdin);
      if(!strncmp(buf,"closed",strlen("closed"))){
          break;
      }
    #endif
    if (g_clt->needBet) {
      betMsg = build_poker_msg(g_clt->clientId, "bet", buf);
      g_clt->betMoney += INIT_BET_MONEY;
    } else if (g_clt->needHeartBeat) {
      time_stamp = time(NULL);
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%d", (int)time_stamp);
      betMsg = build_poker_msg(g_clt->clientId, "heartbeat", buf);
    }

    if (betMsg && strlen(betMsg) > 0) {
      len = write(g_clt->sockFd, betMsg, strlen(betMsg));
      printf("send:%s,len=%d\n", betMsg, len);
      free(betMsg);
      betMsg = NULL;
    }
    time_stamp = time(NULL);
    g_clt->lastMsgTime = time_stamp;
    g_clt->needBet = 0;
    g_clt->needHeartBeat = 0;
    pthread_mutex_unlock(&g_clt->mutex);
  }
}

void *handleRecv(void *arg)
{
  char buf[TMP_BUF_LEN] = {0};
  int num;
  for(;;)
  {
    num = read(g_clt->sockFd, buf, sizeof(buf)-1);
    if(num  == -1 || num == 0)
    {
      handle_error("read error..\n");
      exit(0); 
    }
    else
    {
      printf("recv:%s\n",buf);
      //handleMsg(buf);
      if (g_clt) {
        parseRecvMsg(g_clt->msgBuf, buf, num, handleJsonMsg);
      }
      //fflush(stdout);
      /*
      if(strncmp(buf,"server:close",sizeof(buf)-1) == 0)
      {
        exit(0);
      }
      */
    }
    memset(buf, 0, sizeof(buf)); 
  }

}
#if 0
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
    pthread_mutex_lock(&g_clt->mutex);
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
        pthread_cond_signal(&g_clt->cond);
    }
#endif
    pthread_mutex_unlock(&g_clt->mutex);
    printf("[%s] unlock..\n", __FUNCTION__);
}
#endif
#if 0
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
#endif

void handleJsonMsg(QueueMsg *queue_msg)
{
  int totalMsgNum = 0;
  int ret = -1;
  Poker_Msg_Module *module = NULL;
  if (queue_msg == NULL) {
    return;
  }
  pthread_mutex_lock(&g_clt->mutex);
  module = queue_msg->module;
  if (module) {
    Poker_Msg_Header *head =  module->msgHeader;
    if (head) {
      totalMsgNum = head->msgTotalNum;
      if (totalMsgNum > 1) {
        printf("totalMsgNum = %d\n", totalMsgNum);
      }
      Poker_Msg_Body * body = module->msgBody;
      
      if (body) {
        strncpy(g_clt->action, body->msgType, sizeof(g_clt->action)-1);
        printf("[%s]body->msgType=%s\n", __FUNCTION__, body->msgType);
  #ifdef AUTO_CLIENT
        if (strncmp(body->msgType, "bet", strlen("bet")) == 0) {
          g_clt->needBet = 1;
          g_clt->lastMsgTime = time(NULL);
          printf("pthread_cond_signal start...\n");
          pthread_cond_signal(&g_clt->cond);
        } else if (strncmp(body->msgType, "signal", strlen("signal")) == 0) {
          if (strncmp(body->msgValue, "server:closed", strlen("server:closed")) == 0) {
            ret = raise(SIGINT);
            if( ret != 0 ) 
            {
                printf("错误：不能生成 SIGINT 信号\n");
                //exit(0);
            }
          }
        } else if (strncmp(body->msgType, "money", strlen("money")) == 0) {
           g_clt->betMoney = atoi(body->msgValue);
           printf("betMoney = %d\n", g_clt->betMoney);
        } else if (strncmp(body->msgType, "winer", strlen("winer")) == 0) {
                printf("[%s] recv winer...\n", __FUNCTION__);
                // 取消定时器发送心跳
               setLoopTime(CLIENT_STOP_LOOP_TIME);
        }
  #endif
      }
    }
    if (module) {
        free_poker_msg_module(module);
        module = NULL;
    }
    if (queue_msg) {
      free(queue_msg);
      queue_msg = NULL;
    }
  }
  pthread_mutex_unlock(&g_clt->mutex);
  //printf("[%s] unlock..\n", __FUNCTION__);
}
