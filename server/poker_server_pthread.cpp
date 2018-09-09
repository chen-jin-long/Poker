#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h> 
#include<signal.h>

#include "poker_game.h"
#include "poker_server.h"

#define MAX_LINE 100
#define SERV_PORT 8000

#define POKER_ACTION_LOGIN 0x00000001
#define POKER_ACTION_BET_1 0x00000002
#define POKER_ACTION_BET_2 0x00000003
#define POKER_ACTION_BET_3 0x00000004
#define POKER_ACTION_FLOP  0x00000005
#define POKEK_ACTION_UNKNOWN 0xffffffff

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

//pthread_key_t key; // private data for pthread

char status[3] = {0};
ConnData conn_data;
pthread_rwlock_t rwlock;
int listenfd;
int stage = 0;

POKER_ROOM proom; 
Person person[3];

void * handleMsg(void *arg);
int parseRecvInfo(const char *buf,INFO *info);
void login_process(int conn);
int doInfoAction(Msg *msg);
void stop(int signo);
int findConnectedUser(int conn,ConnData *data);
void sendMsgToAll(const char *data);
void sendMsgToUser(int conn,const char *data);
int isAllLogin(ConnData *data);
void *do_poker_process(void *arg);
int findUserLoc(int conn,ConnData *data);

#define TRUE 1
#define FALSE 0 

int main()
{
    struct sockaddr_in server,client;
    int connectfd, client_addr_len,n;
    socklen_t clientaddr_len;
    //char buf[MAX_LINE];
    //POKER_DESK *pdesk = NULL;
    setupPokerRoom(&proom);
    memset(conn_data.connList,0,sizeof(conn_data.connList));
    pthread_rwlock_init(&rwlock,NULL);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    int mw_optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&mw_optval,sizeof(mw_optval)); 

    signal(SIGINT,stop);
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERV_PORT);

    if(-1 == bind(listenfd,(struct sockaddr *)&server,sizeof(server)))
    {
      handle_error("bind");
    } 
    listen(listenfd,20);
    printf("Accetpting connections..\n");

    while(1){
      clientaddr_len = sizeof(client);
      connectfd = accept(listenfd,(struct sockaddr *)&client,&clientaddr_len);
      if(conn_data.size < 3)
      {
        //conn_data.connList[conn_data.size] = connectfd;
        conn_data.size ++;
      }

      if(connectfd < 0){
        printf("accept error!\n");
        close(listenfd);
        exit(-1);
      }
      //POKER_DESK *pdesk = NULL;
      printf("client connectd..\n");
      pthread_t ptd = -1;
      INFO info;
      info.type = 0;
      info.size = 0;
      memset(info.value,0,sizeof(info.value));
      //Msg msg = {connectfd};
      Msg msg;
      msg.conn = connectfd;
      msg.info = &info;
   
      if(pthread_create(&ptd,NULL,handleMsg,(void *)&msg) < -1)
      {
         printf("pthread_cread failed!\n");
      }
      printf("over....\n");
      //close(connectfd);
  }
  pthread_rwlock_destroy(&rwlock);
  close(listenfd);
  return 0;
}

void *handleMsg(void *arg)
{
   printf("handleMsg...\n");
   int len = 0;
   char buf[MAX_LINE];
   memset(buf,0,MAX_LINE);
   //Msg *msg_temp = (Msg *)arg;
   int conn = ((Msg *)arg)->conn;
   //INFO *info = msg_temp->info;
   INFO info;
   info.type = 0;
   info.size = 0;
   memset(info.value,0,sizeof(info.value));

   Msg msg;
   msg.conn = conn;
   msg.info = &info;
   for(;;)
   {
     if((len = recv(conn,buf,sizeof(buf),0)) > 0)
     {
       printf("info: %s",buf);
       printf("msg.conn: %d ,conn: %d\n",msg.conn,conn);
       parseRecvInfo(buf,&info);
       doInfoAction(&msg);
       send(conn,buf,len,0);
       printf("handleMsg : send to %d\n",conn);
       memset(buf,0,MAX_LINE); 
     } 
   }
  return ((void*)0);
} 

int parseRecvInfo(const char *buf,INFO *info)
{
  char temp[5] = {0};
  strncpy(temp,buf,sizeof(info->type));
  info->type = atoi(temp);   
  printf("type: %d\n",info->type);
  memset(temp,0,sizeof(temp));
  strncpy(temp,buf+sizeof(info->size),sizeof(info->size));
  info->size = atoi(temp);
  strncpy(info->value,buf+sizeof(info->type)+sizeof(info->size),sizeof(info->value));
  return 0;
}

int doInfoAction(Msg *msg)
{
  if(msg->info->type == POKER_ACTION_LOGIN && stage == 0)
  {
    
   if(findConnectedUser(msg->conn,&conn_data) == FALSE)
   {
     login_process(msg->conn);
   }else{
     printf("retry login for connectid: %d\n",msg->conn);
   }
  } 
  else if(stage == 1)
  {
    if(msg->info->type == POKER_ACTION_BET_1)
    {
       printf("POKER_ACTION_BET_1\n");
       int loc = findUserLoc(msg->conn,&conn_data);
       if(loc != -1)
       {
          printf("loc = %d\n",loc);
          status[loc] = 'b';
       }
       else
       {
           printf("error to findUserLoc..\n");

       }

    }

  }
  else if(stage == 2)
  {
       if(msg->info->type == POKER_ACTION_FLOP)
       {
         printf("POKER_ACTION_FLOP\n");
         int loc = findUserLoc(msg->conn,&conn_data);
         if(loc != -1)
         {
            printf("loc = %d\n",loc);
            status[loc] = 'c';
         }

       }
  }

  return 0;
}

void login_process(int conn)
{
  printf("login_process..\n");
  pthread_rwlock_rdlock(&rwlock);
  for(int i = 0;i < 3;i++)
  {
    if(status[i] != 'l')
    {
      pthread_rwlock_unlock(&rwlock);
      pthread_rwlock_wrlock(&rwlock);
      status[i] = 'l';
      conn_data.connList[i] = conn;
     // pthread_rwlock_unlock(&rwlock);
      break;
    }
  }

  if(isAllLogin(&conn_data) == TRUE)
  {
    printf("all users login success...\n");
    //stage = 1;
    pthread_t ptd = -1;
     
    if(pthread_create(&ptd,NULL,do_poker_process,NULL) < -1)
    {
      handle_error("error to create do_poker_process"); 
    }
    //sendMsgToAll("next stage....\n");
    //sleep(3);
    //sendMsgToUser(conn_data.connList[0],"bet...");
  }

 pthread_rwlock_unlock(&rwlock);

}

void *do_poker_process(void *arg)
{
  int cur_conn = -1;
  for(;;)
  {
    if(stage == 0)
    {
       sendMsgToAll("next stage....\n");
       sleep(1);
       sendMsgToUser(conn_data.connList[0],"bet...\n");
       //curr_conn = conn_data.connList[0];
       cur_conn = 0;
       stage = 1;
    }
    else if(stage == 1)
    {

       if(status[cur_conn] == 'b')
       {
         printf("enter send ...\n");
         cur_conn++;
         if(cur_conn < conn_data.size)
         {
           //cur_conn++;
           sendMsgToUser(conn_data.connList[cur_conn],"bet...\n");
         }
         else
         {
            stage = 2;
            sendMsgToAll("next stage 222....\n");
            sleep(1);
            sendMsgToUser(conn_data.connList[0],"bet 2...\n");
            cur_conn = 0;
         }
         
       }

    }
    else if(stage == 2)
    {
        if(status[cur_conn] == 'c')
        {
	  cur_conn++;
          if(cur_conn < conn_data.size)
          {
              sendMsgToUser(conn_data.connList[cur_conn],"flop..\n");
          }
          else
          {
             stage = 3;
             cur_conn = 0;
          }
        }
    }
    sleep(1);
  }

}
void stop(int signo)
{
  printf("stop...\n");
  sendMsgToAll("server:close");
  close(listenfd);
  pthread_rwlock_destroy(&rwlock);
  exit(0);
}

void sendMsgToAll(const char *data)
{
  for(int i=0;i < conn_data.size && i < 3;i++)
  {
    printf("send to %d..\n",conn_data.connList[i]);
    send(conn_data.connList[i],data,strlen(data),0);
  }

}

void sendMsgToUser(int conn,const char *data)
{
  printf("sendMsgToUser conn = %d,len = %d\n",conn,(int)strlen(data));
  int len = send(conn,data,strlen(data),0);
  printf("send : len = %d\n",len);
}

int findConnectedUser(int conn,ConnData *data)
{
  for(int i=0;i < data->size;i++)
  {
    if(data->connList[i] == conn)
    {
      return TRUE; 
    }
  }
  
  return FALSE; 
}

int findUserLoc(int conn,ConnData *data)
{

  for(int i=0;i < data->size;i++)
  {
    if(data->connList[i] == conn)
    {
      return i; 
    }
  }
  return -1;
}

int isAllLogin(ConnData *data)
{
 int result = TRUE;
 for(int i = 0;i < 3;i++)
 {
   if(data->connList[i] == 0)
   {
      result = FALSE;
      break;
   }
 }

return result;
}

void build_msg()
{


}
