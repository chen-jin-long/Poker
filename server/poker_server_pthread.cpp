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
#include "poker_send.h"
#include "common.h"

#define MAX_LINE 100
#define SERV_PORT 8000


#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

//pthread_key_t key; // private data for pthread

char status[3] = {0};
ConnData conn_data;
pthread_rwlock_t rwlock;
int listenfd;
int g_stage = POKER_STAGE_LOGIN;
int g_poker_index = 0;
POKER_ROOM proom; 
Person person[3];
Poker *p_global = NULL;

void * handleMsg(void *arg);
int parseRecvInfo(const char *buf,INFO *info);
void login_process(int conn);
int doInfoAction(Msg *msg);
void stop(int signo);
int findConnectedUser(int conn,ConnData *data);
void sendMsgToAll(const char *data);
void sendPrivPokerToAll();
void sendPrivPoker(int conn);
void sendMsgToUser(int conn,const char *data);
int isAllLogin(ConnData *data);
void *do_poker_process(void *arg);
int findUserLoc(int conn,ConnData *data);
char * build_priv_poker_msg(int conn,int *len);
//void dumpPrivMsg(const char *msg);

#define TRUE 1
#define FALSE 0 

int main()
{
    struct sockaddr_in server,client;
    int connectfd, client_addr_len,n;
    socklen_t clientaddr_len;
    //char buf[MAX_LINE];
    //POKER_DESK *pdesk = NULL;
    p_global = wash_poker();
    /*
    char *priv_msg = NULL;
    int msg_len = 0;
    priv_msg = build_priv_poker_msg(&msg_len);
    */
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
       printf("info: %s,len = %d\n",buf,len);
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
  if(msg->info->type == POKER_ACTION_LOGIN && g_stage == POKER_STAGE_LOGIN)
  {
    
   if(findConnectedUser(msg->conn,&conn_data) == FALSE)
   {
     login_process(msg->conn);
   }else{
     printf("retry login for connectid: %d\n",msg->conn);
   }
  } 
  else if(g_stage == POKER_STAGE_BET)
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
  else if(g_stage == POKER_STAGE_FLOP)
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
    //g_stage = 1;
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
    if(g_stage == POKER_STAGE_LOGIN)
    {
       sendMsgToAll("next stage....\n");
       sendPrivPokerToAll();
       sleep(1);
       sendMsgToUser(conn_data.connList[0],"bet...\n");
       //curr_conn = conn_data.connList[0];
       cur_conn = 0;
       g_stage = POKER_STAGE_BET;
    }
    else if(g_stage == POKER_STAGE_BET)
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
            g_stage = POKER_STAGE_FLOP;
            sendMsgToAll("next stage 222....\n");
            sleep(1);
            sendMsgToUser(conn_data.connList[0],"bet 2...\n");
            cur_conn = 0;
         }
         
       }

    }
    else if(g_stage == POKER_STAGE_FLOP)
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
             g_stage = POKER_STAGE_TURN;
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

void sendPrivPokerToAll()
{
  int i = 0;
  for(i = 0;i < conn_data.size && i < 3;i++)
  {
    printf("send to %d..\n",conn_data.connList[i]);
    sendPrivPoker(conn_data.connList[i]);
    sleep(2);
  }
}

void sendPrivPoker(int conn)
{
    char *priv_msg = NULL;
    int len = 0;
    priv_msg = build_priv_poker_msg(conn,&len);
    printf("priv_msg : %s\n",priv_msg);
    //printf("priv_msg : %2x\n",priv_msg);
   // printf("=======len : %d =======\n",len);
    //if(priv_msg[len-1] == '\n') printf("press enter..\n");
    //dumpPrivMsg(priv_msg);
    int num = send(conn,priv_msg,len,0);
    printf("num = %d\n",num);
    free(priv_msg);
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

char * build_priv_poker_msg(int conn,int *len)
{
  Req_Poker poker = {0};
  poker.command = POKER_ACTION_PRIV;
  poker.user_id = conn;
  poker.len = 2;
  //Poker *p2 = (Poker *)malloc(poker.len * sizeof(Poker));
  Poker *p2 = p_global + g_poker_index;

  *len = sizeof(poker.command) + sizeof(poker.user_id) + sizeof(poker.len) + poker.len * sizeof(Poker) + 1; // +1 for \0
  char * buf = (char *)malloc(*len);
  memset(buf,0,*len);
  int index = 0;
  snprintf(buf,*len,"%04d%04d%04d",poker.command,poker.user_id,poker.len);
  index += 12;
  get_poker(p2,buf+index);
  index += 3;
  g_poker_index++;
  get_poker(p2+g_poker_index,buf+index);
  index += 3;
  g_poker_index++;
  /* 
  memcpy(buf,&poker.command,sizeof(poker.command));
  index += sizeof(poker.command);
  memcpy(buf+index,&poker.user_id,sizeof(poker.user_id));
  index += sizeof(poker.user_id);
  memcpy(buf+index,&poker.len,sizeof(poker.len));
  index += sizeof(poker.len);
  memcpy(buf+index,p2,sizeof(Poker)*poker.len);
  index += (poker.len)*sizeof(Poker);
  for(i = 0;i< *len-1;i++)
  {
     if(buf[i] == 0)
     {
       buf[i] = '0';
     }
     else if(1 <= buf[i] && buf[i] <= 15)
     {
       buf[i] += '0';
     } 
  }
  */
  buf[*len - 1] = '\n';
  return buf;
}

#if 0
void dumpPrivMsg(const char *msg)
{
  Req_Poker poker = {0};
  int index = 0;
  int len = strlen(msg);
  char temp[256] = {0};
  strncpy(temp,msg,sizeof(temp)-1);
  int i = 0;
  for(i = 0;i < len;i++)
  {
    if(temp[i] == '0')
    {
      temp[i] = 0;
    }
  }
  memcpy(&poker.command,temp,sizeof(poker.command));
  index += sizeof(poker.command);
  memcpy(&poker.user_id,temp+index,sizeof(poker.user_id));
  index += sizeof(poker.user_id);
  memcpy(&poker.len,temp+index,sizeof(poker.len));
  index += sizeof(poker.len);
  printf("command : %d ",poker.command);
  printf("user_id: %d ",poker.user_id);
  printf("len: %d \n",poker.len); 
  Poker *p = (Poker *)malloc(poker.len);
  if(p == NULL) return;
  memcpy(p,temp+index,sizeof(Poker)*poker.len); 
  for(i = 0; i < poker.len;i++)
  {
    printf("poker: value: %d ,color = %c\n",(p+i)->value,(p+i)->color); 
  }
  free(p);

}
#endif
