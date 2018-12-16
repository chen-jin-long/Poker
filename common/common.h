#ifndef __POKER_COMMON_COMMON__
#define __POKER_COMMON_COMMON__
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "poker.h"

#define POKER_ACTION_LOGIN 0x00000001
#define POKER_ACTION_PRIV  0x00000002
#define POKER_ACTION_BET_1 0x00000003
#define POKER_ACTION_BET_2 0x00000004
#define POKER_ACTION_BET_3 0x00000005
#define POKER_ACTION_FLOP  0x00000006
#define POKEK_ACTION_UNKNOWN 0xffffffff

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


void dumpPrivMsg(const char *msg);

#ifdef __cplusplus
}
#endif
#endif

