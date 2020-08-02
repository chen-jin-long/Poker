#ifndef __POKER_SERVER__
#define __POKER_SERVER__
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "common.h"

#define USER_ID_LEN 10
typedef struct
{
  int type;
  int size;
  char value[256];

}INFO;

typedef struct msg
{
  int conn;
  INFO *info;
}Msg;

typedef struct
{
  int connList[MAX_DESK_PLAYER];
  int size;
}ConnData;


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
