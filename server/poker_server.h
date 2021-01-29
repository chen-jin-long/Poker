#ifndef __POKER_SERVER__
#define __POKER_SERVER__
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "common.h"
#include "msg_json.h"

typedef struct {
    int clientSN;
    int gameType;
    int roomId;
    int deskId;
    // 玩家在每桌上的索引
    int playerIndex;
} GamePlayerDataBase;

#define USER_ID_LEN 10
/*
typedef struct
{
  int clientSN;
  int type;
  int size;
  char value[256];

}INFO;

typedef struct
{
  int conn;
  //INFO info;
  Poker_Msg_Module *module;
}Msg;
*/

typedef struct
{
  int connList[ALL_PLAYER_NUM_MAX];
  int size;
  PokerMsgBuf *msgBuf[ALL_PLAYER_NUM_MAX];
}ConnData;

/*
typedef struct {
    int stage;
    int action;
    void (*handleActionCmd)(Msg *msg);
    void (*handleStageCmd)(Msg *msg);
}ActionCmd;
*/

#if 0
typedef struct
{
  int command;
  int user_id;
  int flag;
  int action;
  int msg;
}Req_Bet;

typedef struct
{
  int command;
  int user_id;
  int len;
  Poker *poker;
}Req_Poker;

#endif

#ifdef __cplusplus
}
#endif
#endif
