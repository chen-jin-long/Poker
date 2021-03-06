#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h> 
#include<signal.h>
#include<limits.h>

#include "poker_game.h"
#include "poker_server.h"
#include "poker_send.h"
#include "Utils.h"
//#include "poker_server_pthread.h"
//#include "common.h"
#include "thread_pool.h"
#include "msg_json.h"

#define MAX_LINE 256
#define SERV_PORT 8000
#define BROADCAST_CLIENT_SN 0xFFFFFFFF
#define IS_BROADCAST_MSG 1
#define NOT_BROADCAST_MSG 0
#define MAX_HEART_DELTA_TIME 20

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

//pthread_key_t key; // private data for pthread

//int g_status[MAX_DESK_PLAYER] = {0};
ConnData *g_conn_data = NULL;
//pthread_rwlock_t rwlock;
/* 发牌的写锁 */
pthread_rwlock_t dptlock;
int listenfd;
//int g_stage = POKER_STAGE_LOGIN;
POKER_ROOM g_proom[MAX_POKER_ROOM]; 
GamePlayerDataBase *g_playerInfo = NULL;
thread_pool_t *g_msgPool = NULL;

typedef char * (* BUILD_POKER_MSG)(POKER_DESK *desk, Person *person, int *len);

//void * handleMsg(void *arg);
//void * handleAcceptReq(void *arg);
void * handleSelectAcceptReq(void *arg);
void * monitorGameProcess(void *arg);

int getPersonConnState(PersonConn *conn);

//int parseRecvInfo(const char *buf,INFO *info);
void login_process(QueueMsg *queue_msg);
void doInfoAction(QueueMsg *msg);
void stop(int signo);
//int findConnectedUser(int conn,ConnData *data);
void sendMsgToUser(int conn,const char *data);
void sendMsgToAll(POKER_DESK *desk, const char *data);
//void sendPrivPokerToAll();
//void sendPrivPoker(POKER_DESK *desk, Person *person);
void sendPokerMsgToUser(POKER_DESK *desk, Person *person, int action);
void sendPokerMsgToAll(POKER_DESK *desk, int action);
void sendWinerBestChance(Winer *head, POKER_DESK *desk);

int isAllLogin(POKER_DESK *desk);
void do_poker_process(POKER_DESK * desk, int personIndex);
//int findUserLoc(int conn,ConnData *data);
char * build_priv_poker_msg(POKER_DESK *desk, Person *person, int *msgLen);
char * build_flop_poker_msg(POKER_DESK *desk, Person *person,int *msgLen);
char * build_turn_poker_msg(POKER_DESK *desk, Person *person,int *msgLen);
char * build_river_poker_msg(POKER_DESK *desk, Person *person,int *msgLen);
char * build_bestchance_poker_msg(POKER_DESK *desk, Person *person, int *msgLen);
//void dumpPrivMsg(const char *msg);

Poker * dispatchPoker(POKER_DESK *desk, int num);
//int getPokerIndex();
int  registerPlayer(GamePlayerDataBase **allPlayerInfo);
int matchPlayerClientSN(Poker_Msg_Module *module, GamePlayerDataBase * playerInfo);
void doInfoActionByThreadPool(void *param);
int isDeskNextStage(POKER_DESK *desk, int personIndex);
int putMsgInfoToThreadPool(QueueMsg *msg);
void do_send_money_process(POKER_DESK *desk);

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
    pthread_t ptdAccept = -1;
    pthread_t ptdMonitor = -1;
    socklen_t clientaddr_len;
    char ip[INET_ADDRSTRLEN] = {0};
    //char buf[MAX_LINE];
    //POKER_DESK *pdesk = NULL;
    int i , j;
    ret = registerPlayer(&g_playerInfo);
    if (ret != 0) {
        return -1;
    }
    /*
    p_global = wash_poker();
    if (p_global == NULL) {
        printf("g_global is NULL, so can't run server.\n");
        return -1;
    }
    */
    /*
    char *priv_msg = NULL;
    int msg_len = 0;
    priv_msg = build_priv_poker_msg(&msg_len);
    */
    setupPokerRoom(g_proom);
    //memcpy(g_game_pub, dispatchPoker(PUB_LEN), sizeof(Poker) * PUB_LEN);
    //InitGamePubPoker(g_proom->pdesk[0]->game, &g_game_pub);
    g_conn_data = (ConnData *)malloc(sizeof(ConnData));
    if (g_conn_data) {
        memset(g_conn_data, 0, sizeof(ConnData));
        memset(g_conn_data->connList,0,sizeof(g_conn_data->connList));
    }

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
    inet_ntop(AF_INET, &server.sin_addr, ip, sizeof(ip));
    printf("new client IP is (%s)\n", ip);
    if(-1 == bind(listenfd,(struct sockaddr *)&server,sizeof(server))) {
        handle_error("bind");
    } 
    if (-1 == listen(listenfd,MAX_DESK_PLAYER*MAX_POKER_DESK*MAX_POKER_ROOM)) {
        handle_error("listen");
    }

    if (pthread_create(&ptdMonitor, NULL, monitorGameProcess, g_proom)) {
        printf("pthread_create monitor failed!\n");
        exit(-1);
    }

    printf("Accetpting connections..\n");
    if (pthread_create(&ptdAccept, NULL, handleSelectAcceptReq, &listenfd)) {
        printf("pthread create accept failed!\n");
        exit(-1);
    }

    while(1) {
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

void updateBetMoney(POKER_DESK *desk, unsigned int money) 
{
    printf("[%s]", __FUNCTION__);
    if (money >= desk->betMoney) {
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


int parseRecvInfo(const char *buf,INFO *info)
{

    char temp[MAX_LINE] = {0};
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
#endif
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
/*
const char *g_betInfo[] = {
    "init", "login", "priv", "bet", "flop", "turn", "river", "over",
};
*/

const char * g_pokerStrAction[POKER_ACTION_MAX] = {
  [POKER_ACTION_INIT] = "init",
  [POKER_ACTION_LOGIN] =  "login",
  [POKER_ACTION_PRIV] = "priv",
  [POKER_ACTION_BET] = "bet",
  [POKER_ACTION_FLOP] = "flop",
  [POKER_ACTION_TURN] = "turn",
  [POKER_ACTION_RIVER] = "river",
  [POKER_ACTION_OVER] = "over",
  [POKER_ACTION_WINER] = "winer",
  [POKER_ACTION_SIGNAL] = "signal",
  [POKER_ACTION_MONEY] = "money",
  [POKER_ACTION_HEARTBEAT] = "heartbeat",
};


int getNextValidPlayerIndex(POKER_DESK *desk, int personIndex, int *index)
{
    int i = 0;
    if (desk == NULL || index == NULL) {
        return -1;
    }

    for (i = personIndex +  1; i < MAX_DESK_PLAYER; i++) {
        if (getPersonConnState(&desk->psnConn[i]) == POKER_CONN_ENABLE) {
            *index = i;
            return 0;
        }
    }

    return -1;
}

void do_player_process(POKER_DESK * desk, int personIndex)
{
    char info[MAX_LINE] = {0};
    char *dataMsg = NULL;
    int index = -1;
    /* 沃日，这里的sleep怎么影响这么大，多线程编程测试通过不等于完全正确，有可能条件不满足，无法暴露 */
    //sleep(2);

    /* 当所有玩家都bet后，控制进入下一个阶段, 也可以根据personIndex是否为最后一个(MAX_POKER_DESK-1)来判断是否进入下一个阶段 */
    if (isDeskNextStage(desk, personIndex) == 0) {
        // 所有玩家的状态一致时进入下一阶段，stage递增一个
        ++desk->stage;
        if (desk->stage == POKER_ACTION_OVER) {
            return;
        } else {
            sendPokerMsgToAll(desk, desk->stage);
            index = 0;
            //desk->person[index].status = desk->stage;
        }
    } else {
        /* 控制下一个玩家的动作 */
        if (getNextValidPlayerIndex(desk, personIndex, &index) != 0) {
            printf("[%s] getNextValidPlayerIndex error.\n", __FUNCTION__);
            return;
        }
    }

    if (index < 0 || index >= MAX_DESK_PLAYER) {
        printf("[%s] error index=%d\n", __FUNCTION__, index);
    }
    snprintf(info, sizeof(info), "please bet for %s\n", g_pokerStrAction[desk->stage]);
    dataMsg = build_poker_msg(desk->person[index].clientSN, g_pokerStrAction[POKER_ACTION_BET], info);
    if (dataMsg) {
        sendMsgToUser(desk->person[index].connId, dataMsg);
        free(dataMsg);
        dataMsg = NULL;
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

void doInfoAction(QueueMsg *msg)
{
    //int i = 0;
    if(msg == NULL) {
        return;
    }
    printf("[%s]...\n", __FUNCTION__);
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
    //return 0;
}

int putMsgInfoToThreadPool(QueueMsg *msg)
{
    if(msg == NULL) {
        return -1;
    }
    queue_push(g_msgPool->queue, msg);
    return 0;
}

void doInfoActionByThreadPool(void *param)
{
    QueueMsg *msg = (QueueMsg *)param;
    if (msg) {
        //printf("get: %d\n", *num);
        login_process(msg);
    }
}

int do_login_dispatch_poker(POKER_DESK *desk, int connId, int clientSN)
{
    int playerDeskIndex = -1;
    int i = 0;
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        Person *person = &desk->person[i];
        if(person->status == POKER_ACTION_INIT)
        {
            playerDeskIndex = i;
            //person->status = POKER_ACTION_LOGIN;
            person->connId = connId;
            person->clientSN = clientSN;
            memcpy(&(person->priv[0]), dispatchPoker(desk, 1), sizeof(Poker));
            memcpy(&(person->priv[1]), dispatchPoker(desk, 1), sizeof(Poker));
            //person->status = POKER_ACTION_PRIV;
            printf("[%s] person[%d]->status = %d\n", __FUNCTION__, i, desk->person[i].status);
            return playerDeskIndex;
        }
    }
    return -1;
}

void login_process(QueueMsg *queue_msg)
{
    //char info[MAX_LINE] = {0};
    printf("%s..\n", __FUNCTION__);
    //pthread_rwlock_rdlock(&rwlock);
    int i = 0, roomId = 0, deskId = 0;
    GamePlayerDataBase *player = NULL;
    POKER_DESK *desk = NULL;
    PersonConn *conn = NULL;

    if (queue_msg == NULL) {
        printf("[%s] msg is NULL.\n", __FUNCTION__);
    }

    if (queue_msg->module == NULL) {
        printf("[%s] msg->module is NULL.\n", __FUNCTION__);
    }
    /***
     *  从clientSN 转化为数据库中的索引, 在转换为玩家在每桌中的索引
     *  clientSN --> person_db_index --> playerDeskIndex(playerIndex)
     *
    ***/
    /* 注册处理，默认已经注册 */
    int person_db_index = matchPlayerClientSN(queue_msg->module, g_playerInfo);

    if (person_db_index != DEFAULT_PLAYER_IN_DESK_POSITION) {
        player = &g_playerInfo[person_db_index];
        if (player == NULL) {
            return;
        }
        roomId = player->roomId;
        deskId = player->deskId;
        //playerIndex = player->playerIndex;
        desk = g_proom[roomId].pdesk[deskId];
    } else {
        printf("err to match clientSn, please login in.\n");
        return;
    }

    pthread_rwlock_wrlock(&desk->deskLock);
    Poker_Msg_Body * body = queue_msg->module->msgBody;
    Poker_Msg_Header *header = queue_msg->module->msgHeader;
    if (body && header) {
        //strncpy(g_clt->action, body->msgType, sizeof(g_clt->action)-1);
        if (strncmp(body->msgType, g_pokerStrAction[POKER_ACTION_LOGIN], strlen(g_pokerStrAction[POKER_ACTION_LOGIN])) == 0) {
            player->playerIndex = do_login_dispatch_poker(desk, queue_msg->connId, header->clientSN);
            //printf("status = %d\n", desk->person[0].status);
            //playerIndex = player->playerIndex;
            desk->person[player->playerIndex].status = POKER_ACTION_LOGIN;
            desk->psnConn[player->playerIndex].loginTime = time(NULL);
        } else if (strncmp(body->msgType, g_pokerStrAction[POKER_ACTION_BET], strlen(g_pokerStrAction[POKER_ACTION_BET])) == 0) {
            // 控制玩家的状态
            desk->person[player->playerIndex].status = desk->stage;
            int money = atoi(body->msgValue);
            updateBetMoney(desk, (unsigned int)money);
            do_send_money_process(desk);
            //do_poker_process(desk, playerIndex);
        } else if (strncmp(body->msgType, g_pokerStrAction[POKER_ACTION_HEARTBEAT], strlen(g_pokerStrAction[POKER_ACTION_HEARTBEAT])) == 0) {
            printf("[%s] heartBeat.\n", __FUNCTION__);
            conn = &desk->psnConn[player->playerIndex];
            if (conn) {
                pthread_rwlock_wrlock(&conn->stateLock);
                conn->heartCount++;
                if (conn->heartCount == LONG_MAX) {
                    conn->heartCount=1;
                }
                conn->heartTime = (time_t)atol(body->msgValue);
                printf("[%s]conn addr=%p, curTime=%ld\n", __FUNCTION__, conn, time(NULL));
                printf("[%s]conn->heartTime=%ld, player->playerIndex=%d\n", __FUNCTION__, conn->heartTime, player->playerIndex);
                pthread_rwlock_unlock(&conn->stateLock);
            }
            goto end_process;
        } else {
            printf("[%s] no msgtype:%s\n", __FUNCTION__, body->msgType);
        }
        if (isAllLogin(desk) == TRUE) {
            printf("all users login success...\n");
            conn = &desk->psnConn[player->playerIndex];
            if (conn) {
                pthread_rwlock_wrlock(&conn->stateLock);
                conn->lasgMsgTime = time(NULL);
                pthread_rwlock_unlock(&conn->stateLock);
            }
            do_poker_process(desk, player->playerIndex);
        }
    }
end_process:
    pthread_rwlock_unlock(&desk->deskLock);

    if (queue_msg) {
        if (queue_msg->module) {
            free_poker_msg_module(queue_msg->module);
            queue_msg->module = NULL;
        }
        free(queue_msg);
        queue_msg = NULL;
    }
}

void do_poker_judge_winer(POKER_DESK *desk)
{
    int i = 0, result = 0;
    printf("[%s] start....\n", __FUNCTION__);
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        if (getPersonConnState(&desk->psnConn[i]) == POKER_CONN_ENABLE) {
            result = fast_poker_algo(desk->person[i], desk->game);
            printf("[poker result] loc=%d, result = %d\n", i, result);
            sort_poker_bestChance(desk->person[i].best_chance);
            set_bestChance_color(&desk->person[i], desk->game, result);
            desk->person[i].gameScore = result;
            print_BestChance((Poker *)(desk->person[i].best_chance), PUB_LEN);
        }
    }
    Winer *head = NULL;
    head = getGameWiner(&desk->person[0]);
    if (head) {
        printWiner(head, desk);
        sendWinerBestChance(head, desk);
        freeAllWiner(head);
    } else {
        printf("====Winer is NULL====\n");
    }
    //desk->stage = POKER_STAGE_OVER;
}

void do_send_priv_process(POKER_DESK *desk) {
    int i = 0;
    for(i = 0; i < MAX_DESK_PLAYER; i++) {
        //sendPrivPoker(desk, &(desk->person[i]));
        if (getPersonConnState(&desk->psnConn[i]) == POKER_CONN_ENABLE) {
            sendPokerMsgToUser(desk, &desk->person[i], POKER_ACTION_PRIV);
            desk->person[i].status = POKER_ACTION_PRIV;
        } else {
            printf("[%s] connection heartbeat timeout.\n", __FUNCTION__);
        }
    }
}

void do_send_money_process(POKER_DESK *desk) {
    char info[MAX_LINE] = {0};
    char *dataMsg = NULL;
    snprintf(info, sizeof(info),"%d", desk->betMoney);
    dataMsg = build_poker_msg(BROADCAST_CLIENT_SN, g_pokerStrAction[POKER_ACTION_MONEY], info);
    if (dataMsg) {
        sendMsgToAll(desk, dataMsg);
        free(dataMsg);
        dataMsg = NULL;
    }
}
void do_poker_process(POKER_DESK *desk, int personIndex)
{
    if (desk->stage >= POKER_ACTION_LOGIN && desk->stage <= POKER_STAGE_OVER) {
        if (desk->stage == POKER_ACTION_LOGIN) {
            do_send_priv_process(desk);
            desk->stage = POKER_ACTION_PRIV;
            //发送初始Money
            do_send_money_process(desk);
            //desk->stage = POKER_ACTION_BET;
        } 
        if(desk->stage >= POKER_ACTION_PRIV && desk->stage < POKER_ACTION_OVER) {
            do_player_process(desk, personIndex);
        }
        if (desk->stage == POKER_STAGE_OVER) {
            do_poker_judge_winer(desk);
        }
    } else {
        printf("[%s] stage[%d] is override.\n", __FUNCTION__, desk->stage);
    }
}

void stop(int signo)
{
    int i = 0, j = 0;
    printf("stop...\n");
    for (i = 0; i < MAX_POKER_ROOM; i++) {
        for (j = 0; j < MAX_POKER_DESK; j++) {
            char *msg = build_poker_msg(BROADCAST_CLIENT_SN, g_pokerStrAction[POKER_ACTION_SIGNAL], "server:close");
            if (msg) {
               sendMsgToAll(g_proom[i].pdesk[j], msg);
               free(msg);
               msg = NULL;
            }
            //sendMsgToAll(g_proom[i].pdesk[j], "server:close");
            pthread_rwlock_destroy(&g_proom[i].pdesk[j]->deskLock);
        }
    }

    close(listenfd);

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
  for(i = 0;i < g_conn_data->size && i < MAX_DESK_PLAYER;i++)
  {
    printf("send to %d..\n",g_conn_data->connList[i]);
    sendPrivPoker(g_conn_data->connList[i]);
    sleep(2);
  }
}
#endif

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
    case POKER_ACTION_WINER:
      msg = build_bestchance_poker_msg;
      break;
    default:
      msg = NULL;
      break;
  }
  return msg;
}

/*
void sendPrivPoker(POKER_DESK *desk, Person *person)
{
    char *priv_msg = NULL;
    int len = 0;
    priv_msg = build_priv_poker_msg(desk, person, &len);
    printf("priv_msg : %s, len = %d\n",priv_msg, len);
    char *msgData = build_poker_msg(person->clientSN, g_pokerStrAction[POKER_ACTION_PRIV], priv_msg);
    //printf("priv_msg : %2x\n",priv_msg);
   // printf("=======len : %d =======\n",len);
    //if(priv_msg[len-1] == '\n') printf("press enter..\n");
    //dumpPrivMsg(priv_msg);
    if (msgData) {
        len = strlen(msgData);
        int num = send(person->connId, msgData, len, 0);
        printf("connId = %d, num = %d\n",person->connId, num);
        free(msgData);
        msgData = NULL;
    }
    if (priv_msg) {
        free(priv_msg);
        priv_msg = NULL;
    }
}
*/

void sendMsgToUser(int conn,const char *data)
{
    if (conn != -1) {
        printf("sendMsgToUser conn = %d,len = %d, data=%s\n",conn,(int)strlen(data), data);
        int len = send(conn,data,strlen(data),0);
        printf("send : len = %d\n",len);
    }
}

void sendMsgToAll(POKER_DESK *desk, const char *data)
{
  int i = 0;
  for(i = 0; i < MAX_DESK_PLAYER; i++)
  { 
    //printf("send to %d, msg=%s\n",g_conn_data->connList[i], data);
    //send(desk->person[i].connId, data, strlen(data), 0);
      sendMsgToUser(desk->person[i].connId, data);
  }

}

char *getMsgData(int isBroadCast, POKER_DESK *desk, Person *person, int action)
{
    char *info = NULL;
    char *msgData = NULL;
    int len = 0, clientSN = -1;
    BUILD_POKER_MSG poker_msg = getPokerMsgByType(action);
    if (poker_msg == NULL) {
        printf("[%s] poker_msg is NULL.\n", __FUNCTION__);
        return NULL;
    }
    info = poker_msg(desk, person, &len);
    if (info == NULL) {
         printf("[%s] info is NULL.\n", __FUNCTION__);
        return NULL;
    }
    if (isBroadCast == IS_BROADCAST_MSG) {
        clientSN = BROADCAST_CLIENT_SN;
    } else {
        clientSN = person->clientSN;
    }
    msgData = build_poker_msg(clientSN, g_pokerStrAction[action], info);
    if (info) {
        free(info);
        info = NULL;
    }
    return msgData;
}

void sendPokerMsgToUser(POKER_DESK *desk, Person *person, int action)
{
    char *msgData = getMsgData(NOT_BROADCAST_MSG, desk, person, action);
    if (msgData) {
        sendMsgToUser(person->connId, msgData);
        free(msgData);
        msgData = NULL;

    }
}

void sendPokerMsgToAll(POKER_DESK *desk, int action)
{
  int i = 0;
  for(i = 0; i < MAX_DESK_PLAYER; i++)
  {
    //printf("send to %d..\n",g_conn_data->connList[i]);
    //sendPrivPoker(g_conn_data->connList[i]);
    sendPokerMsgToUser(desk, &desk->person[i], action);
  }
}

void sendWinerBestChance(Winer *head, POKER_DESK *desk)
{
    Winer *cur = head;
    Person *person = NULL;
    char *msgData = NULL;
    while(cur){
        person = &desk->person[cur->index];
        msgData = getMsgData(IS_BROADCAST_MSG, desk, person, POKER_ACTION_WINER);
        if (msgData) {
            sendMsgToAll(desk, msgData);
            free(msgData);
            msgData = NULL;
        } else {
            printf("[%s] msgData is NULL.\n", __FUNCTION__);
        }
        cur = cur->next;
    }
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
        if(desk->person[i].status == POKER_ACTION_INIT)
        {
            result = FALSE;
            break;
        }
    }
    printf("[%s]\n", __FUNCTION__);
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
 
char * build_priv_poker_msg(POKER_DESK *desk, Person *person, int *msgLen)
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

char * build_flop_poker_msg(POKER_DESK *desk, Person *person, int *msgLen)
{
    if (desk == NULL || desk->game == NULL) {
        return NULL;
    }
    //int loc = findUserLoc(conn,&g_conn_data);
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_FLOP;
    head.user_id = person->clientSN;
    head.len = POKER_FLOP_NUM;
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub, POKER_FLOP_NUM);
        get_poker_msg(*desk->game->pub, POKER_FLOP_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
    }
    return buf;
}

char * build_turn_poker_msg(POKER_DESK *desk, Person *person, int *msgLen)
{
    if (desk == NULL || desk->game == NULL) {
        return NULL;
    }
    //int loc = findUserLoc(conn,&g_conn_data);
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_TURN;
    head.user_id = person->clientSN;
    head.len = POKER_TURN_NUM;
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub + POKER_FLOP_NUM, POKER_TURN_NUM);
        get_poker_msg(*desk->game->pub+POKER_FLOP_NUM, POKER_TURN_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
    }
    return buf;
}

char * build_river_poker_msg(POKER_DESK *desk, Person *person, int *msgLen)
{
    //int loc = findUserLoc(conn,&g_conn_data);
    if (desk == NULL || desk->game == NULL) {
        return NULL;
    }
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_RIVER;
    head.user_id = person->clientSN;
    head.len = POKER_RIVER_NUM;
    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub+POKER_FLOP_NUM+POKER_TURN_NUM, POKER_RIVER_NUM);
        get_poker_msg(*desk->game->pub+POKER_FLOP_NUM+POKER_TURN_NUM, POKER_RIVER_NUM, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
    }
    return buf;
}

char * build_bestchance_poker_msg(POKER_DESK *desk, Person *person, int *msgLen)
{
    Req_Poker_Header head = {0};
    head.command = POKER_ACTION_WINER;
    head.user_id = person->clientSN;
    head.len = PUB_LEN;

    char *buf = getReqPokerMsg(&head, msgLen);
    if (buf) {
        //memcpy(buf+REQ_POKER_HEADER_LEN, g_game_pub+POKER_FLOP_NUM+POKER_TURN_NUM, POKER_RIVER_NUM);
        get_poker_msg(*person->best_chance, PUB_LEN, buf+REQ_POKER_HEADER_LEN);
        buf[*msgLen - 1] = '\n';
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

    for(i = 0; i < g_conn_data->size; i++) {
        if (FD_ISSET(g_conn_data->connList[i], &read_fds)) {
            connId = g_conn_data->connList[i];
            if((len = recv(connId, buf,sizeof(buf),0)) > 0)
            {
                printf("info:%s,len = %d\n", buf, len);
                g_conn_data->msgBuf[i]->connId = connId;
                parseRecvMsg(g_conn_data->msgBuf[i], buf, len, doInfoAction);
                 /*
                QueueMsg *queue_msg = (QueueMsg *)malloc(sizeof(QueueMsg));
                if (queue_msg != NULL) {
                    queue_msg->conn = connId;
                    //parseRecvInfo(buf, &msg->info);
                    //doInfoAction(msg);
                }
                */
                //send(connId, buf, len, 0);
                //printf("[%s]:send to %d\n", __FUNCTION__, connId);
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
    int clientFd[ALL_PLAYER_NUM_MAX];
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

        for (i = 0; i < ALL_PLAYER_NUM_MAX; i++) {
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
            if (index < ALL_PLAYER_NUM_MAX) {
                clientFd[index] = client_sockfd;
                index++;
                if (g_conn_data) {
                    g_conn_data->connList[g_conn_data->size] = client_sockfd;
                    if (g_conn_data->msgBuf[g_conn_data->size] == NULL) {
                        g_conn_data->msgBuf[g_conn_data->size] = (PokerMsgBuf *)malloc(sizeof(PokerMsgBuf));
                        memset(g_conn_data->msgBuf[g_conn_data->size], 0, sizeof(PokerMsgBuf));
                        //g_conn_data->msgBuf[g_conn_data->size]->sockFd = client_sockfd;
                    }                   
                    g_conn_data->size++;
                }
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

int getPersonConnState(PersonConn *conn)
{
    pthread_rwlock_rdlock(&conn->stateLock);
    if (conn->heartState == POKER_CONN_ENABLE) {
        pthread_rwlock_unlock(&conn->stateLock);
        return POKER_CONN_ENABLE;
    }
    pthread_rwlock_unlock(&conn->stateLock);
    return POKER_CONN_DISABLE;
}

void * monitorGameProcess(void *arg)
{
    POKER_ROOM *room = NULL;
    int roomIndex = 0;
    int deskIndex = 0;
    int playerIndex = 0;
    int deltaTime = 0;
    time_t curTime = 0;

    PersonConn *conn = NULL;
    if (arg) {
        room = (POKER_ROOM *)arg;
    } else {
        pthread_exit(0);
    }
    while (1) {
        for (roomIndex = 0; roomIndex < MAX_POKER_ROOM; roomIndex++) {
            for (deskIndex = 0; deskIndex < MAX_POKER_DESK; deskIndex++) {
                for (playerIndex = 0; playerIndex < MAX_DESK_PLAYER; playerIndex++) {
                    if (room[roomIndex].pdesk[deskIndex]->person[playerIndex].status >= POKER_ACTION_LOGIN) {
                        conn = &(room[roomIndex].pdesk[deskIndex]->psnConn[playerIndex]);
                        if (conn && conn->heartCount > 0) {
                            pthread_rwlock_wrlock(&conn->stateLock);
                            curTime = time(NULL);
                            //printf("[%s-%ld]conn addr=%p, heartTime=%ld\n", __FUNCTION__, curTime, conn, conn->heartTime);
                            deltaTime = abs(curTime - conn->heartTime);
                            conn->heartState = (deltaTime > MAX_HEART_DELTA_TIME) ? POKER_CONN_DISABLE:POKER_CONN_ENABLE;
                            if (conn->heartState == POKER_CONN_DISABLE && conn->heartTime > 0) {
                                //printf("Room[%d].Desk[%d].Player[%d].heartState=%d, curTime=%ld, heartTime=%ld\n", roomIndex, deskIndex, playerIndex, conn->heartState, curTime, conn->heartTime);
                            }
                            curTime = 0;
                            pthread_rwlock_unlock(&conn->stateLock);
                        }
                        conn = NULL;
                    }
                }
            }
        }
        //sleep(5);
    }
    return NULL;
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
       playerDB[i].roomId = 0;
    }
    return 0;
}

int matchPlayerClientSN(Poker_Msg_Module *module, GamePlayerDataBase * playerInfo)
{
    int i = 0;
    if (module == NULL || playerInfo == NULL) {
        return -1;
    }
    for (i = 0 ; i < ALL_PLAYER_NUM_MAX; i++) {
        //if (strncmp(playerInfo[i].clientSN, msg->info->clientSN, CLIENT_SN_LEN) == 0) {
        if (module->msgHeader && (playerInfo[i].clientSN == module->msgHeader->clientSN)) {
            return i;
        }
    }
    return -1;
}

int isDeskNextStage(POKER_DESK *desk, int personIndex)
{
    if (personIndex == MAX_DESK_PLAYER - 1 &&  desk->person[personIndex].status != desk->stage) {
        printf("[%s]person[%d].status = %d\n", __FUNCTION__, personIndex , desk->person[personIndex].status);
    } else {
        int i = 0;
        for(i = personIndex + 1; i < MAX_DESK_PLAYER; i++)
        {
            if (getPersonConnState(&desk->psnConn[i]) == POKER_CONN_ENABLE && desk->person[i].status != desk->stage) {
                printf("[%s]person[%d].status = %d\n", __FUNCTION__, i , desk->person[i].status);
                return -1;
            }
        }
    }

    return 0;
}



