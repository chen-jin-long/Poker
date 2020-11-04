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
#include "thread_pool.h"

#define MAX_LINE 100
#define SERV_PORT 8000
#define ALL_PLAYER_NUM_MAX (MAX_POKER_ROOM * MAX_POKER_DESK *MAX_DESK_PLAYER)

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

//pthread_key_t key; // private data for pthread

//int g_status[MAX_DESK_PLAYER] = {0};
ConnData g_conn_data;
//pthread_rwlock_t rwlock;
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
thread_pool_t *g_msgPool = NULL;

typedef char * (* BUILD_POKER_MSG)(int conn,int *len);

//void * handleMsg(void *arg);
//void * handleAcceptReq(void *arg);
void * handleSelectAcceptReq(void *arg);
int parseRecvInfo(const char *buf,INFO *info);
void login_process(Msg *msg);
int doInfoAction(Msg *msg);
void stop(int signo);
//int findConnectedUser(int conn,ConnData *data);
void sendMsgToUser(int conn,const char *data);
void sendMsgToAll(POKER_DESK *desk, const char *data);
//void sendPrivPokerToAll();
void sendPrivPoker(Person *person);
void sendPokerMsgToUser(int conn, int action);
void sendPokerMsgToAll(POKER_DESK *desk, int action);

int isAllLogin(POKER_DESK *desk);
void do_poker_process(POKER_DESK * desk, int personIndex);
//int findUserLoc(int conn,ConnData *data);
char * build_priv_poker_msg(Person *person, int *msgLen);
char * build_flop_poker_msg(int conn,int *len);
char * build_turn_poker_msg(int conn,int *len);
char * build_river_poker_msg(int conn,int *len);
//void dumpPrivMsg(const char *msg);

Poker * dispatchPoker(int num);
//int getPokerIndex();
int  registerPlayer(GamePlayerDataBase **allPlayerInfo);
int matchPlayerClientSN(Msg *msg, GamePlayerDataBase * playerInfo);
void doInfoActionByThreadPool(void *param);
int isDeskNextStage(POKER_DESK *desk, int action);
int putMsgInfoToThreadPool(Msg *msg);


#define MAX_POKER_MSG_THREAD_POOL_SIZE 10
#define DEFAULT_PLAYER_IN_DESK_POSITION -1
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
    int i , j;
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

    ret = create_thread_pool(&g_msgPool, MAX_POKER_MSG_THREAD_POOL_SIZE, doInfoActionByThreadPool);
    if (ret != 0) {
        printf("[%s]failed.\n", __FUNCTION__);
        return -1;
    }

    //pthread_rwlock_init(&rwlock,NULL);
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

    for (i = 0; i < MAX_POKER_ROOM; i++) {
        for (j = 0; j < MAX_POKER_DESK; j++) {
            pthread_rwlock_destroy(&g_proom[i].pdesk[j]->deskLock);
        }
    }

    pthread_rwlock_destroy(&dptlock);
    close(listenfd);
    return 0;
}

//unsigned int g_money = 100;

void updateBetMoney(POKER_DESK *desk, int playerDeskId, Msg *msg) 
{
    unsigned int money = (unsigned int)atoi(msg->info.value);
    desk->person[playerDeskId].status = msg->info.type;
    if (money >=  desk->betMoney) {
        desk->betMoney= money;
    }
}

#if 0
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
#endif

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

#if 0
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
#endif

const char *g_betInfo[] = {
    "init", "login", "priv", "bet", "flop", "turn", "river", "over",
};

void handleQueueMsg(POKER_DESK * desk, int personIndex)
{
    char info[256] = {0};
    int action = desk->person[personIndex].status;
    printf("enter send g_status =%d\n", action);
    snprintf(info, sizeof(info),"[bet]:g_money=%d\n", desk->betMoney);
    sendMsgToAll(desk, info);
    memset(info, 0, sizeof(info));
    snprintf(info, sizeof(info), "please bet for %s...", g_betInfo[desk->stage]);
    sendMsgToUser(desk->person[personIndex].connId, info);
    if (isDeskNextStage(desk, action) == 0 && action != POKER_ACTION_RIVER)
    {
        desk->stage = desk->stage + 1;
        memset(info, 0, sizeof(info));
        snprintf(info, sizeof(info), "next stage : %s....\n", g_betInfo[desk->stage]);
        sendMsgToAll(desk, info);
        sendPokerMsgToAll(desk, action + 1);
        memset(info, 0, sizeof(info));
        snprintf(info, sizeof(info), "please bet for %s...\n", g_betInfo[desk->stage]);
        sendMsgToUser(desk->person[0].connId, info);
    }
}

/*
ActionCmd g_actionCmds [] = {
    { POKER_STAGE_LOGIN,    POKER_ACTION_LOGIN    , handleLoginCmd,   NULL  },
    { POKER_STAGE_PRIV,     POKER_ACTION_PRIV     , handleBetCmd,     NULL  },
    { POKER_STAGE_BET,      POKER_ACTION_BET      , handleBetCmd,     NULL  },
    { POKER_STAGE_FLOP,     POKER_ACTION_FLOP     , handleBetCmd,     NULL  },
    { POKER_STAGE_TURN,     POKER_ACTION_TURN     , handleBetCmd,     NULL  },
    { POKER_STAGE_RIVER,    POKER_ACTION_RIVER    , handleBetCmd,     NULL  },
    { POKER_STAGE_OVER,     POKER_ACTION_OVER     , handleBetCmd,     NULL  },
    { POKER_STAGE_UNKNOWN,  POKEK_ACTION_UNKNOWN  , NULL,             NULL  },
};
*/

int doInfoAction(Msg *msg)
{
    //int i = 0;
    if(msg == NULL) {
        return -1;
    }
    putMsgInfoToThreadPool(msg);

    #if 0
    for(i = 0; i < sizeof(g_actionCmds)/sizeof(ActionCmd); i++) {
        if (g_actionCmds[i].stage == g_proom[0].pdesk[0]->stage && g_actionCmds[i].action == msg->info->type) {
            printf("[%s]stage = %d, action = %d\n", __FUNCTION__, g_proom[0].pdesk[0]->stage, msg->info->type);
            g_actionCmds[i].handleActionCmd(msg);
            break;
        }
    }
    #endif
    return 0;
}

int putMsgInfoToThreadPool(Msg *msg)
{
    if(msg == NULL) {
        return -1;
    }
    queue_push(g_msgPool->queue, msg);
    return 0;
}

void doInfoActionByThreadPool(void *param)
{
    Msg *info = (Msg *)param;
    if (info) {
        //printf("get: %d\n", *num);
        login_process(info);
    }
}

int do_login_dispatch_poker(POKER_DESK *desk, Msg *msg)
{
    int i = 0;
    int playerDeskId = -1;
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        Person person = desk->person[i];
        if(person.status == POKER_ACTION_INIT)
        {
            playerDeskId= i;
            person.status = POKER_ACTION_LOGIN;
            person.connId = msg->conn;
            person.clientSN = msg->info.clientSN;
            memcpy(&(person.priv[0]), dispatchPoker(1), sizeof(Poker));
            memcpy(&(person.priv[1]), dispatchPoker(1), sizeof(Poker));
            person.status = POKER_ACTION_PRIV;
            return playerDeskId;
        }
    }
    return -1;
}

void login_process(Msg *msg)
{
    printf("%s..\n", __FUNCTION__);
    //pthread_rwlock_rdlock(&rwlock);
    int i = 0, roomId = 0, deskId = 0;
    int playerIndex = -1;
    GamePlayerDataBase *player = NULL;
    POKER_DESK *desk = NULL;
    //pthread_rwlock_unlock(&rwlock);
    //pthread_rwlock_wrlock(&rwlock);
    int person_db_index = matchPlayerClientSN(msg, g_playerInfo);

    if (person_db_index != -1) {
        player = &g_playerInfo[person_db_index];
        roomId = player->roomId;
        deskId = player->deskId;
        playerIndex = player->playerIndex;
        desk = g_proom[roomId].pdesk[deskId];
    } else {
        printf("err to match clientSn, please login in.\n");
        return;
    }

    pthread_rwlock_wrlock(&desk->deskLock);
    
    if (playerIndex != DEFAULT_PLAYER_IN_DESK_POSITION && isAllLogin(desk) == TRUE) {
        printf("all users login success...\n");
        updateBetMoney(desk, playerIndex, msg);
        do_poker_process(desk, playerIndex);
    } else {
        if (playerIndex == DEFAULT_PLAYER_IN_DESK_POSITION) {
            player->playerIndex = do_login_dispatch_poker(desk, msg);
            playerIndex = player->playerIndex;
            sendPrivPoker(&(desk->person[playerIndex]));
        } else {
            printf("[%s] person_db_index = %d have loginned.\n", __FUNCTION__, person_db_index);
        }
    }

    pthread_rwlock_unlock(&desk->deskLock);
}

void do_poker_judge_winer(POKER_DESK *desk)
{
    int i = 0, result = 0;
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        result = fast_poker_algo(desk->person[i], desk->game);
        printf("[poker result] loc=%d, result = %d\n", i, result);
        sort_poker_bestChance(desk->person[i].best_chance);
        set_bestChance_color(&desk->person[i], desk->game, result);
        desk->person[i].gameScore = result;
        print_BestChance((Poker *)(desk->person[i].best_chance), PUB_LEN);
    }
    Winer *head = NULL;
    head = getGameWiner(&desk->person[0]);
    if (head) {
        printWiner(head);
        freeAllWiner(head);
    } else {
        printf("====Winer is NULL====\n");
    }
    desk->stage = POKER_STAGE_OVER;
}

void do_poker_process(POKER_DESK * desk, int personIndex)
{
    if(desk->stage != POKER_STAGE_OVER) {
       handleQueueMsg(desk, personIndex);
    } else {
       do_poker_judge_winer(desk); 
    }
}

void stop(int signo)
{
    int i = 0, j = 0;
    printf("stop...\n");
    if (p_global) {
        free(p_global);
        p_global = NULL;
    }
    for (i = 0; i < MAX_POKER_ROOM; i++) {
        for (j = 0; j < MAX_POKER_DESK; j++) {
            sendMsgToAll(g_proom[i].pdesk[j], "server:close");
            pthread_rwlock_destroy(&g_proom[i].pdesk[j]->deskLock);
        }
    }

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

    if (g_playerInfo) {
        free(g_playerInfo);
        g_playerInfo = NULL;
    }
    exit(0);
}


#if 0
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
#endif

BUILD_POKER_MSG getPokerMsgByType(int action)
{
  BUILD_POKER_MSG msg = NULL;
  switch (action)
  {
    //case POKER_ACTION_PRIV:
     // msg = build_priv_poker_msg;
      //break;
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

void sendPrivPoker(Person *person)
{
    char *priv_msg = NULL;
    int len = 0;
    priv_msg = build_priv_poker_msg(person, &len);
    printf("priv_msg : %s\n",priv_msg);
    //printf("priv_msg : %2x\n",priv_msg);
   // printf("=======len : %d =======\n",len);
    //if(priv_msg[len-1] == '\n') printf("press enter..\n");
    //dumpPrivMsg(priv_msg);
    int num = send(person->connId, priv_msg, len, 0);
    printf("num = %d\n",num);
    free(priv_msg);
}

void sendMsgToUser(int conn,const char *data)
{
  printf("sendMsgToUser conn = %d,len = %d\n",conn,(int)strlen(data));
  int len = send(conn,data,strlen(data),0);
  printf("send : len = %d\n",len);
}

void sendMsgToAll(POKER_DESK *desk, const char *data)
{
  int i = 0;
  for(i = 0; i < MAX_DESK_PLAYER; i++)
  { 
    //printf("send to %d, msg=%s\n",g_conn_data.connList[i], data);
    //send(desk->person[i].connId, data, strlen(data), 0);
      sendMsgToUser(desk->person[i].connId, data);
  }

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
    sendMsgToUser(conn, msg);
    if (action == POKER_ACTION_PRIV) {
       free(msg);
    }

}

void sendPokerMsgToAll(POKER_DESK *desk, int action)
{
  int i = 0;
  for(i = 0; i < MAX_DESK_PLAYER; i++)
  {
    //printf("send to %d..\n",g_conn_data.connList[i]);
    //sendPrivPoker(g_conn_data.connList[i]);
    sendPokerMsgToUser(desk->person[i].connId, action);
  }
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

/*
int getPokerIndex()
{
  pthread_rwlock_rdlock(&dptlock);
  int index = g_poker_index;
  pthread_rwlock_unlock(&dptlock);
  return index;
}
*/


#if 0
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
#endif

int isAllLogin(POKER_DESK *desk)
{
    int result = TRUE, i = 0;
    for(i = 0; i < MAX_DESK_PLAYER; i++)
    {
        int status = desk->person[i].status;
        if(status == POKER_ACTION_PRIV)
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
 
char * build_priv_poker_msg(Person *person, int *msgLen)
{
  //int loc = findUserLoc(conn,&g_conn_data);
  Req_Poker_Header head = {0};
  head.command = POKER_ACTION_PRIV;
  head.user_id = person->clientSN;
  head.len = PRIV_LEN;
  char *buf = getReqPokerMsg(&head, msgLen);
  if (buf) {
    //memcpy(buf+REQ_POKER_HEADER_LEN, &g_proom[0].pdesk[0]->person[loc].priv[0], sizeof(Poker)*PRIV_LEN);
    get_poker_msg(&person->priv[0], head.len, buf+REQ_POKER_HEADER_LEN);
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

void handleRecvMsg(void *arg)
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
            if((len = recv(connId, buf,sizeof(buf),0)) > 0)
            {
                printf("info: %s,len = %d\n", buf, len);
                Msg *msg = (Msg *)malloc(sizeof(Msg));
                if (msg != NULL) {
                    msg->conn = connId;
                    parseRecvInfo(buf, &msg->info);
                    doInfoAction(msg);
                }
                send(connId, buf, len, 0);
                printf("[%s]:send to %d\n", __FUNCTION__, connId);
                memset(buf, 0, MAX_LINE); 
            }
        }
    }
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
       handleRecvMsg(&read_fds);
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
       playerDB[i].deskId = i/MAX_DESK_PLAYER;
       playerDB[i].playerIndex = DEFAULT_PLAYER_IN_DESK_POSITION;
    }
    return 0;
}

int matchPlayerClientSN(Msg *msg, GamePlayerDataBase * playerInfo)
{
    int i = 0;
    if (msg == NULL || playerInfo == NULL) {
        return -1;
    }
    for (i = 0 ; i < ALL_PLAYER_NUM_MAX; i++) {
        //if (strncmp(playerInfo[i].clientSN, msg->info->clientSN, CLIENT_SN_LEN) == 0) {
        if (playerInfo[i].clientSN == msg->info.clientSN) {
            return i;
        }
    }
    return -1;
}

int isDeskNextStage(POKER_DESK *desk, int action)
{
    int i = 0;
    for(i = 0; i < MAX_DESK_PLAYER; i++)
    {
        if (desk->person[i].status != action) {
            return 0;
        }
    }
    return -1;
}


