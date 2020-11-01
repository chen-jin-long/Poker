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
//#include "common.h"
#include "Utils.h"
#include "poker_server_pthread.h"

#define MAX_LINE 100
#define SERV_PORT 8000
#define ALL_PLAYER_NUM_MAX (MAX_POKER_ROOM * MAX_POKER_DESK *MAX_DESK_PLAYER)

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

//pthread_key_t key; // private data for pthread

//int g_status[MAX_DESK_PLAYER] = {0};
ConnData g_conn_data;
pthread_rwlock_t rwlock;
/* 发牌的写锁 */
pthread_rwlock_t dptlock;
int listenfd;
//int g_stage = POKER_STAGE_LOGIN;
int g_poker_index = 0;
POKER_ROOM g_proom[MAX_POKER_ROOM]; 
//Person person[MAX_DESK_PLAYER];
Poker *p_global = NULL;
Poker g_game_pub[PUB_LEN];
GamePlayerDataBase *g_playerInfo = NULL;
char *g_flop_msg = NULL;
char *g_turn_msg = NULL;
char *g_river_msg = NULL;

typedef char * (* BUILD_POKER_MSG)(int conn,int *len);

void * handleMsg(void *arg);
void * handleAcceptReq(void *arg);
void * handleSelectAcceptReq(void *arg);
int parseRecvInfo(const char *buf,INFO *info);
void login_process(Msg *msg);
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
char * build_flop_poker_msg(int conn,int *len);
char * build_turn_poker_msg(int conn,int *len);
char * build_river_poker_msg(int conn,int *len);
//void dumpPrivMsg(const char *msg);
void sendPokerMsgToUser(int conn, int action);
void sendPokerMsgToAll(int action);
Poker * dispatchPoker(int num);
int getPokerIndex();
int  registerPlayer(GamePlayerDataBase **allPlayerInfo);
int matchPlayerClientSN(Msg *msg, GamePlayerDataBase * playerInfo);


#define TRUE 1
#define FALSE 0 


/*******消息格式_V1.1******

clientSN+command+len+value
%4d%4d0004%4d


**************************/

int main()
{
    struct sockaddr_in server,client;
    int connectfd, client_addr_len, ret = -1;
    pthread_t ptdAccept = -1;;
    socklen_t clientaddr_len;
    //char buf[MAX_LINE];
    //POKER_DESK *pdesk = NULL;
    ret = registerPlayer(&g_playerInfo);
    if (ret != 0) {
        return -1;
    }
    p_global = wash_poker();
    if (p_global == NULL) {
        printf("g_global is NULL, so can't run server.\n");
        return -1;
    }
    /*
    char *priv_msg = NULL;
    int msg_len = 0;
    priv_msg = build_priv_poker_msg(&msg_len);
    */
    setupPokerRoom(g_proom);
    memcpy(g_game_pub, dispatchPoker(PUB_LEN), sizeof(Poker) * PUB_LEN);
    InitGamePubPoker(g_proom->pdesk[0]->game, &g_game_pub);
    memset(g_conn_data.connList,0,sizeof(g_conn_data.connList));
    pthread_rwlock_init(&rwlock,NULL);
    pthread_rwlock_init(&dptlock, NULL);
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
    if (-1 == listen(listenfd,MAX_DESK_PLAYER*MAX_POKER_DESK*MAX_POKER_ROOM))
    {
        handle_error("listen");
    }

    printf("Accetpting connections..\n");
    if (pthread_create(&ptdAccept, NULL, handleSelectAcceptReq, &listenfd)) {
        printf("pthread create accept failed!\n");
        exit(-1);
    }

    while(1)
    {
       sleep(2);
    }

    pthread_rwlock_destroy(&rwlock);
    pthread_rwlock_destroy(&dptlock);
    close(listenfd);
    return 0;
}

//unsigned int g_money = 100;

void updateBetMoney(unsigned int money) {
  if (money >=  g_proom[0].pdesk[0]->betMoney) {
    g_proom[0].pdesk[0]->betMoney = money;
  }
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
    strncpy(temp, buf, sizeof(info->clientSN));
    info->clientSN = atoi(temp);
    printf("[%s]clientSN: %d\n", __FUNCTION__, info->clientSN);
    memset(temp, 0, sizeof(temp));

    strncpy(temp,buf+sizeof(info->clientSN),sizeof(info->type));
    info->type = atoi(temp);
    printf("[%s] type: %d\n", __FUNCTION__, info->type);
    memset(temp,0,sizeof(temp));

    strncpy(temp,buf+sizeof(info->clientSN)+sizeof(info->size),sizeof(info->size));
    info->size = atoi(temp);
    memset(info->value, 0 ,sizeof(info->size));

    strncpy(info->value,buf+sizeof(info->clientSN)+sizeof(info->type)+sizeof(info->size),sizeof(info->value));
    return 0;
}

void handleBetCmd(Msg *msg)
{
   unsigned int money = (unsigned int)atoi(msg->info->value);
   int loc = findUserLoc(msg->conn,&g_conn_data);
   if(loc != -1) {
      printf("loc = %d\n",loc);
      g_proom[0].pdesk[0]->person[loc].status = msg->info->type;
      updateBetMoney(money);
   } else {
       printf("error to findUserLoc..\n");
   }
}

void handleLoginCmd(Msg *msg)
{
    if(matchPlayerClientSN(msg, g_playerInfo) == 0 && findConnectedUser(msg->conn,&g_conn_data) == TRUE) {
        login_process(msg);
    } else {
         printf("not login for connectid: %d\n",msg->conn);
    }
}


ActionCmd g_actionCmds [] = {
    { POKER_STAGE_LOGIN,    POKER_ACTION_LOGIN    , handleLoginCmd,   NULL },
    { POKER_STAGE_PRIV,     POKER_ACTION_PRIV     , handleBetCmd,     NULL },
    { POKER_STAGE_BET,      POKER_ACTION_BET      , handleBetCmd,     NULL },
    { POKER_STAGE_FLOP,     POKER_ACTION_FLOP     , handleBetCmd,     NULL },
    { POKER_STAGE_TURN,     POKER_ACTION_TURN     , handleBetCmd,     NULL },
    { POKER_STAGE_RIVER,    POKER_ACTION_RIVER    , handleBetCmd,     NULL },
    { POKER_STAGE_OVER,     POKER_ACTION_OVER     , handleBetCmd,     NULL },
    { POKER_STAGE_UNKNOWN,  POKEK_ACTION_UNKNOWN  , NULL,             NULL },
};


int doInfoAction(Msg *msg)
{
    int i = 0;
    if(msg == NULL && msg->info == NULL) {
        return -1;
    }
    for(i = 0; i < sizeof(g_actionCmds)/sizeof(ActionCmd); i++) {
        if (g_actionCmds[i].stage == g_proom[0].pdesk[0]->stage && g_actionCmds[i].action == msg->info->type) {
            printf("[%s]stage = %d, action = %d\n", __FUNCTION__, g_proom[0].pdesk[0]->stage, msg->info->type);
            g_actionCmds[i].handleActionCmd(msg);
            break;
        }
    }
    return 0;
}

void login_process(Msg *msg)
{
  printf("login_process..\n");
  pthread_rwlock_rdlock(&rwlock);
  int i = 0;
  for(i = 0;i < MAX_DESK_PLAYER;i++)
  {
    if(g_proom[0].pdesk[0]->person[i].status == POKER_ACTION_LOGIN)
    {
      pthread_rwlock_unlock(&rwlock);
      pthread_rwlock_wrlock(&rwlock);
      g_proom[0].pdesk[0]->person[i].status = POKER_ACTION_PRIV;
     // g_conn_data.connList[i] = conn;
      g_proom[0].pdesk[0]->person[i].connId = msg->conn;
      g_proom[0].pdesk[0]->person[i].id = msg->info->clientSN;
      memcpy(&(g_proom[0].pdesk[0]->person[i].priv[0]), dispatchPoker(1), sizeof(Poker));
      memcpy(&(g_proom[0].pdesk[0]->person[i].priv[1]), dispatchPoker(1), sizeof(Poker));
     // pthread_rwlock_unlock(&rwlock);
      break;
    }
  }

  if(isAllLogin(&g_conn_data) == TRUE)
  {
    printf("all users login success...\n");
    //g_proom[0].pdesk[0]->stage = 1;
    pthread_t ptd = -1;
     
    if(pthread_create(&ptd,NULL,do_poker_process,NULL) < -1)
    {
      handle_error("error to create do_poker_process"); 
    }
    //sendMsgToAll("next stage....\n");
    //sleep(3);
    //sendMsgToUser(g_conn_data.connList[0],"bet...");
  }

 pthread_rwlock_unlock(&rwlock);

}

void *do_poker_process(void *arg)
{
  int cur_conn = 0;
  int loc = 0;
  int result = 0;
  char info[256] = {0};
  for(;;)
  {
    if(g_proom[0].pdesk[0]->stage == POKER_STAGE_LOGIN)
    {
       if(g_proom[0].pdesk[0]->person[cur_conn].status == POKER_ACTION_PRIV) {
           sendMsgToAll("next stage....\n");
           //sendPrivPokerToAll();
           sendPokerMsgToAll(POKER_ACTION_PRIV);
           sleep(1);
           snprintf(info, sizeof(info), "[bet]gmoney=%d\n", g_proom[0].pdesk[0]->betMoney);
           sendMsgToAll(info);
           memset(info, 0, sizeof(info));
           sendMsgToUser(g_conn_data.connList[0],"please bet for bet...\n");
           //curr_conn = g_conn_data.connList[0];
           cur_conn = 0;
           g_proom[0].pdesk[0]->stage = POKER_STAGE_BET;
        }
    }
    else if(g_proom[0].pdesk[0]->stage == POKER_STAGE_BET)
    {

       if(g_proom[0].pdesk[0]->person[cur_conn].status == POKER_ACTION_BET)
       {
         printf("enter send g_status = POKER_ACTION_BET...\n");
         cur_conn++;
         if(cur_conn < g_conn_data.size)
         {
           snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
           sendMsgToAll(info);
           sendMsgToUser(g_conn_data.connList[cur_conn], "please bet for bet...\n");
           memset(info, 0, sizeof(info));
           //cur_conn++;
         }
         else
         {
            snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
            sendMsgToAll(info);
            g_proom[0].pdesk[0]->stage = POKER_STAGE_FLOP;
            sendMsgToAll("next stage : flop....\n");
            sleep(2);
            sendPokerMsgToAll(POKER_ACTION_FLOP);
            sendMsgToUser(g_conn_data.connList[0],"please bet for flop...\n");
            cur_conn = 0;
         }
         
       }

    }
    else if(g_proom[0].pdesk[0]->stage == POKER_STAGE_FLOP)
    {
        if(g_proom[0].pdesk[0]->person[cur_conn].status == POKER_ACTION_FLOP)
        {
            cur_conn++;
            if(cur_conn < g_conn_data.size)
            {
              snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
              sendMsgToAll(info);
              sendMsgToUser(g_conn_data.connList[cur_conn], "please bet for flop...\n");
              memset(info, 0, sizeof(info));
              //cur_conn++;
            }
            else
            {
                snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
                sendMsgToAll(info);
                g_proom[0].pdesk[0]->stage = POKER_STAGE_TURN;
                sendMsgToAll("next stage : turn....\n");
                sleep(2);
                sendPokerMsgToAll(POKER_ACTION_TURN);
                sendMsgToUser(g_conn_data.connList[0],"please bet for turn...\n");
                cur_conn = 0;
            }
        }
    }
    else if(g_proom[0].pdesk[0]->stage == POKER_STAGE_TURN)
    {
        if(g_proom[0].pdesk[0]->person[cur_conn].status == POKER_ACTION_TURN)
        {
            cur_conn++;
            if(cur_conn < g_conn_data.size)
            {
              snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
              sendMsgToAll(info);
              sendMsgToUser(g_conn_data.connList[cur_conn], "please bet for turn ...\n");
              memset(info, 0, sizeof(info));
              //cur_conn++;
            }
            else
            {
                snprintf(info, sizeof(info),"[bet]:g_money=%d\n", g_proom[0].pdesk[0]->betMoney);
                sendMsgToAll(info);
                g_proom[0].pdesk[0]->stage = POKER_STAGE_RIVER;
                sendMsgToAll("next stage : river....\n");
                sleep(2);
                sendPokerMsgToAll(POKER_ACTION_RIVER);
                sendMsgToUser(g_conn_data.connList[0],"please bet for river...\n");
                cur_conn = 0;
            }
        }
    }
    else if(g_proom[0].pdesk[0]->stage == POKER_STAGE_RIVER)
    {
        if(g_proom[0].pdesk[0]->person[cur_conn].status == POKER_ACTION_RIVER)
        {
          printf("[%s]  stage = %d, cur_conn = %d\n", __FUNCTION__, g_proom[0].pdesk[0]->stage, cur_conn);
          if (cur_conn >= 0 && cur_conn < MAX_DESK_PLAYER) {
              result = fast_poker_algo(g_proom[0].pdesk[0]->person[cur_conn], g_proom[0].pdesk[0]->game);
              printf("[poker result] loc=%d, result = %d\n", cur_conn, result);
              //print_BestChance((Poker *)(g_proom[0].pdesk[0]->person[cur_conn].best_chance), PUB_LEN);
              sort_poker_bestChance(g_proom[0].pdesk[0]->person[cur_conn].best_chance);
              set_bestChance_color(&g_proom[0].pdesk[0]->person[cur_conn], g_proom[0].pdesk[0]->game, result);
              g_proom[0].pdesk[0]->person[cur_conn].gameScore = result;
              print_BestChance((Poker *)(g_proom[0].pdesk[0]->person[cur_conn].best_chance), PUB_LEN);
          }
          cur_conn++;
          sendPokerMsgToAll(POKER_ACTION_RIVER);
          if(cur_conn < g_conn_data.size)
          {
              sendMsgToUser(g_conn_data.connList[cur_conn],"please bet for river..\n");
          }
          else
          {
             Winer *head = NULL;
             head = getGameWiner(&g_proom[0].pdesk[0]->person[0]);
             if (head) {
                //printf("====winer connId:%d, Score:%d===\n", head->index, head->score);
                printWiner(head);
                freeAllWiner(head);
             } else {
                printf("====Winer is NULL====\n");
             }
             g_proom[0].pdesk[0]->stage = POKER_STAGE_OVER;
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
  if (p_global) {
    free(p_global);
    p_global = NULL;
  }
  sendMsgToAll("server:close");
  close(listenfd);
  if (g_flop_msg) {
    free(g_flop_msg);
    g_flop_msg = NULL;
  }
  if (g_turn_msg) {
    free(g_turn_msg);
    g_turn_msg = NULL;
  }
  if (g_river_msg) {
    free(g_river_msg);
    g_river_msg = NULL;
  }
  pthread_rwlock_destroy(&rwlock);

  if (g_playerInfo) {
    free(g_playerInfo);
    g_playerInfo = NULL;
  }
  exit(0);
}

void sendMsgToAll(const char *data)
{
  int i = 0;
  for(i = 0; i < g_conn_data.size && i < MAX_DESK_PLAYER; i++)
  { 
    printf("send to %d, msg=%s\n",g_conn_data.connList[i], data);
    send(g_conn_data.connList[i],data,strlen(data),0);
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
  for(i = 0;i < g_conn_data.size && i < MAX_DESK_PLAYER;i++)
  {
    printf("send to %d..\n",g_conn_data.connList[i]);
    sendPrivPoker(g_conn_data.connList[i]);
    sleep(2);
  }
}

BUILD_POKER_MSG getPokerMsgByType(int action)
{
  BUILD_POKER_MSG msg = NULL;
  switch (action)
  {
    case POKER_ACTION_PRIV:
      msg = build_priv_poker_msg;
      break;
    case POKER_ACTION_FLOP:
      msg = build_flop_poker_msg;
      break;
    case POKER_ACTION_TURN:
      msg = build_turn_poker_msg;
      break;
    case POKER_ACTION_RIVER:
      msg = build_river_poker_msg;
      break;
    default:
      msg = NULL;
      break;
  }
  return msg;
}

Poker * dispatchPoker(int num)
{
    pthread_rwlock_wrlock(&dptlock);
    if (g_poker_index < 0) {
        printf("[%s] g_poker_index = %d\n", __FUNCTION__, g_poker_index);
        return NULL;
    }
    if (g_poker_index + num > ONE_UNIT_POKER) {
        printf("[%s] no enough poker num, g_poker_index = %d, num =%d\n", __FUNCTION__, g_poker_index, num);
        return NULL;
    }
    Poker *curr = p_global + g_poker_index;
    g_poker_index += num;
    pthread_rwlock_unlock(&dptlock);
    return curr;
}

int getPokerIndex()
{
  pthread_rwlock_rdlock(&dptlock);
  int index = g_poker_index;
  pthread_rwlock_unlock(&dptlock);
  return index;
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

void sendPokerMsgToUser(int conn, int action)
{
    char *msg = NULL;
    int len = 0;
    BUILD_POKER_MSG poker_msg = getPokerMsgByType(action);
    if (poker_msg == NULL) {
      printf("action[%d] can not get method..\n", action);
      return;
    }
    msg = poker_msg(conn,&len);
    printf("[poker msg] : %s\n",msg); 
    int num = send(conn, msg,strlen(msg),0);
    printf("num = %d\n",num);
    if (action == POKER_ACTION_PRIV) {
       free(msg);
    }

}

void sendPokerMsgToAll(int action)
{
  int i = 0;
  for(i = 0;i < g_conn_data.size && i < MAX_DESK_PLAYER;i++)
  {
    printf("send to %d..\n",g_conn_data.connList[i]);
    //sendPrivPoker(g_conn_data.connList[i]);
    sendPokerMsgToUser(g_conn_data.connList[i], action);
    sleep(2);
  }
}

int findConnectedUser(int conn,ConnData *data)
{
  int i = 0;
  for(i=0;i < data->size;i++)
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
  int i = 0;
  for(i = 0; i < data->size; i++)
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
 for(int i = 0;i < MAX_DESK_PLAYER;i++)
 {
   if(data->connList[i] == 0)
   {
      result = FALSE;
      break;
   }
 }

return result;
}

char *getReqPokerMsg(Req_Poker_Header *head, int *msgLen)
{
    int len = sizeof(head->command) + sizeof(head->user_id) + sizeof(head->len) + head->len * sizeof(Poker) + 1; // +1 for \0
    char * buf = (char *)malloc(len);
    if (buf) {
        memset(buf,0,len);
        snprintf(buf,len,"%04d%04d%04d", head->user_id, head->command, head->len);
        *msgLen = len;
        return buf;
    }
    return NULL;
}
 
char * build_priv_poker_msg(int conn,int *msgLen)
{
  int loc = findUserLoc(conn,&g_conn_data);
  Req_Poker_Header head = {0};
  head.command = POKER_ACTION_PRIV;
  head.user_id = loc;
  head.len = PRIV_LEN;
  char *buf = getReqPokerMsg(&head, msgLen);
  if (buf) {
    //memcpy(buf+REQ_POKER_HEADER_LEN, &g_proom[0].pdesk[0]->person[loc].priv[0], sizeof(Poker)*PRIV_LEN);
    get_poker_msg(&g_proom[0].pdesk[0]->person[loc].priv[0], head.len, buf+REQ_POKER_HEADER_LEN);
    buf[*msgLen - 1] = '\n';
  }
  return buf;
}

char * build_flop_poker_msg(int conn,int *msgLen)
{
    if (g_flop_msg) {
        return g_flop_msg;
    }
    //int loc = findUserLoc(conn,&g_conn_data);
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_FLOP;
    head.user_id = 0xff;
    head.len = POKER_FLOP_NUM;
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub, POKER_FLOP_NUM);
        get_poker_msg(g_game_pub, POKER_FLOP_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
        g_flop_msg = buf;
    }
    return buf;
}

char * build_turn_poker_msg(int conn,int *msgLen)
{
    if (g_turn_msg) {
        return g_turn_msg;
    }
    //int loc = findUserLoc(conn,&g_conn_data);
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_TURN;
    head.user_id = 0xff;
    head.len = POKER_TURN_NUM;
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub + POKER_FLOP_NUM, POKER_TURN_NUM);
        get_poker_msg(g_game_pub+POKER_FLOP_NUM, POKER_TURN_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
        g_turn_msg = buf;
    }
    return buf;
}

char * build_river_poker_msg(int conn,int *msgLen)
{
    //int loc = findUserLoc(conn,&g_conn_data);
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_RIVER;
    head.user_id = 0xff;
    head.len = POKER_RIVER_NUM;
    if (g_river_msg) {
        return g_river_msg;
    }
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub+POKER_FLOP_NUM+POKER_TURN_NUM, POKER_RIVER_NUM);
        get_poker_msg(g_game_pub+POKER_FLOP_NUM+POKER_TURN_NUM, POKER_RIVER_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
        g_river_msg = buf;
    }
    return buf;
}

void handleRecvReq(void *arg)
{
   char buf[MAX_LINE] = {0};
   int len = 0, connId = 0, i = 0;
   fd_set read_fds;
   if (arg) {
     read_fds = *((fd_set *)arg);
   } else {
        printf("%s arg is NULL\n", __FUNCTION__);
        return;
   }

    for(i = 0; i < g_conn_data.size; i++) {
        if (FD_ISSET(g_conn_data.connList[i], &read_fds)) {
            connId = g_conn_data.connList[i];
            //INFO info = { 0, 0, {0}};
            INFO info = {0};
            Msg msg = { connId, &info};
            if((len = recv(connId, buf,sizeof(buf),0)) > 0)
            {
                printf("info: %s,len = %d\n", buf, len);
                printf("msg.conn: %d ,conn: %d\n", msg.conn, connId);
                parseRecvInfo(buf, &info);
                doInfoAction(&msg);
                send(connId, buf, len, 0);
                printf("handleMsg : send to %d\n",connId);
                memset(buf, 0, MAX_LINE); 
            }
        }
    }
} 

void * handleAcceptReq(void *arg)
{
    struct sockaddr_in client;
    int connectfd = -1;
    while(1) {
        socklen_t clientaddr_len = sizeof(client);
        connectfd = accept(listenfd,(struct sockaddr *)&client, &clientaddr_len);
        if(g_conn_data.size < MAX_DESK_PLAYER)
        {
            g_conn_data.connList[g_conn_data.size] = connectfd;
            g_conn_data.size++;
        }

        if(connectfd < 0) {
            printf("accept error!\n");
            close(listenfd);
            exit(-1);
        }
        printf("client[%d], connectfd = %d \n", g_conn_data.size-1, connectfd);
        handleRecvReq(NULL);
   }
    return ((void*)0);
}

void * handleSelectAcceptReq(void *arg)
{
    fd_set read_fds;
    fd_set all_fds;
    int client_sockfd, maxFd, server_sockfd = -1;
    int clientFd[MAX_DESK_PLAYER];
    int result = -1, index = 0, i = 0;
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int * param = (int *)arg;
    if(param) {
        server_sockfd = *param;
    } else {
        printf("arg is NULL, so no server_sockfd.\n");
        return NULL;
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&all_fds);
    maxFd = server_sockfd;
    FD_SET(server_sockfd, &read_fds);//将服务器端socket加入到集合中

    while(1) {
        FD_ZERO(&read_fds);
        FD_SET(server_sockfd, &read_fds);

        for (i = 0; i < MAX_DESK_PLAYER; i++) {
            if (clientFd[i] > 0) {
                FD_SET(clientFd[i], &read_fds);
            }
        }
        result = select(maxFd + 1, &read_fds, NULL,  NULL, NULL);
        if (result == -1) {
            printf("select error!\n");
            continue;
        } 
        if (FD_ISSET(server_sockfd, &read_fds)) {
            printf("read_fds is set.\n");
            client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);
            if (client_sockfd == -1) {
                printf("accept error, client_sockfd = %d\n", client_sockfd);
                continue;
            }
            if (index < MAX_DESK_PLAYER) {
                clientFd[index] = client_sockfd;
                index++;
                g_conn_data.connList[g_conn_data.size] = client_sockfd;
                g_conn_data.size++;
            } else {
                printf("index = %d, is bigger than %d\n", index, MAX_DESK_PLAYER);
                continue;
            }
            maxFd = (client_sockfd > maxFd ? client_sockfd : maxFd);
            printf("client_sockfd=%d\n", client_sockfd);
        }
       // handleRecvMsgNoThread();
       handleRecvReq(&read_fds);
    }
    return  NULL;
}

int  registerPlayer(GamePlayerDataBase **allPlayerInfo)
{
    int i = 0;
    if (allPlayerInfo == NULL) {
        return -1;
    }
    int playerMaxNum = sizeof(GamePlayerDataBase) * ALL_PLAYER_NUM_MAX;
    GamePlayerDataBase * playerDB = (GamePlayerDataBase *) malloc(playerMaxNum);
    if (playerDB == NULL) {
        return -1;
    }
    *allPlayerInfo = playerDB;
    memset(playerDB, 0, playerMaxNum);
    for (i = 0; i < ALL_PLAYER_NUM_MAX; i++) {
       //snprintf(playerDB[i].clientSN, CLIENT_SN_LEN+1, "%8d", i);
       playerDB[i].clientSN = i;
    }
    return 0;
}

int matchPlayerClientSN(Msg *msg, GamePlayerDataBase * playerInfo)
{
    int i = 0;
    if (msg == NULL || msg->info == NULL || playerInfo == NULL) {
        return -1;
    }
    for (i = 0 ; i < ALL_PLAYER_NUM_MAX; i++) {
        //if (strncmp(playerInfo[i].clientSN, msg->info->clientSN, CLIENT_SN_LEN) == 0) {
        if (playerInfo[i].clientSN ==  msg->info->clientSN) {
            return 0;
        }
    }
    return -1;
}
