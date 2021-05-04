#ifndef __POKER_COMMON_COMMON__
#define __POKER_COMMON_COMMON__
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "poker.h"
#include "msg_json.h"

#define CLIENT_SN_LEN sizeof(int)
#define MAX_RECV_LEN 2048


#define POKER_ACTION_INIT  0
#define POKER_ACTION_LOGIN 0x00000001
#define POKER_ACTION_PRIV  0x00000002
#define POKER_ACTION_BET 0x00000003
#define POKER_ACTION_FLOP  0x00000004
#define POKER_ACTION_TURN  0x00000005
#define POKER_ACTION_RIVER 0x00000006
#define POKER_ACTION_OVER  0x00000007
#define POKER_ACTION_WINER 0x00000008
#define POKER_ACTION_SIGNAL 0x00000009
#define POKER_ACTION_MONEY  0x0000000A
#define POKER_ACTION_HEARTBEAT 0x0000000B
#define POKER_ACTION_MAX       0x0000000C
#define POKEK_ACTION_UNKNOWN 0xffffffff


#define POKER_CONN_ENABLE 1
#define POKER_CONN_DISABLE 0

typedef enum {
  POKER_STAGE_LOGIN = 1,
  POKER_STAGE_PRIV,
  POKER_STAGE_BET,
  POKER_STAGE_FLOP,
  POKER_STAGE_TURN,
  POKER_STAGE_RIVER,
  POKER_STAGE_OVER,
  POKER_STAGE_UNKNOWN = 0xff,
}POKER_STAGE;


typedef struct
{
  int user_id;
  int command;
  int flag;
  int action;
  int msg;
}Req_Bet;

typedef struct {
    int user_id;
    int command;
    int len; 
}Req_Poker_Header;

typedef struct
{
  Req_Poker_Header *head;
  Poker *poker;
}Req_Poker;

typedef enum
{
  POKER_OK = 0,
  POKER_PARAM_NULL_ERROR,
  POKER_ARRAY_OVERLOAD_ERROR,
  POKER_MALLOC_FAILED,

}POKER_RETURE_PARAM;


typedef struct {
  char remain_buf[MAX_RECV_LEN];
  int remain_len;
  //在Server端使用，对Client端意义不大
  int connId;
} PokerMsgBuf;


typedef struct {
  int clientId;
  char action[128];
  int sockFd;
  //char remain_buf[MAX_RECV_LEN];
  //int remain_len;
  PokerMsgBuf *msgBuf;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int betMoney;
  int needBet;
  int needHeartBeat;
  time_t lastMsgTime;
} Client;

void dumpPrivMsg(const char *msg);
POKER_RETURE_PARAM get_poker(Poker *poker,char *out);
POKER_RETURE_PARAM get_poker_msg(Poker *poker, int num, char *out);
#define REQ_POKER_HEADER_LEN (sizeof(Req_Poker_Header))

typedef void (* HandlerJsonMsgCb)(QueueMsg *queue_msg);
void parseRecvMsg(PokerMsgBuf *msgBuf, char *recv_buf, int recv_len, HandlerJsonMsgCb handler);

#ifdef __cplusplus
}
#endif
#endif

