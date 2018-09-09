#include<stdio.h>

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
  int connList[3];
  int size;
}ConnData;

typedef struct
{
  int command;
  int user_id[USER_ID_LEN];
  int flag;
  int action;
  int msg;
}Req_Bet;

typedef struct
{
  int command;
  int user_id[USER_ID_LEN];
  int len;
  Poker *poker;
}Req_Poker;
