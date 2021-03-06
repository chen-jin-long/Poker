#ifndef _POKER_POKER_
#define _POKER_POKER_

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include "poker_type.h"
//#include "Utils.h"

#define PRIV_LEN 2
#define PUB_LEN  5
#define ONE_UNIT_POKER 54
#define ONE_UNIT_DK_POKER 52
#define POKER_FLOP_NUM 3
#define POKER_RIVER_NUM 1
#define POKER_TURN_NUM 1

typedef struct {
  int value;
  char color;  // 's'黑桃 ; 'h' 红心 ; 'c' 梅花 ; 'd' 方块
}Poker;

typedef struct {
 int clientSN;
 Poker priv[PRIV_LEN];
 Poker (*best_chance)[PUB_LEN]; //最优解
 int status;
 int connId;
 int gameScore;
}Person;

typedef struct {
  //Person p[2];
  Poker (*pub)[PUB_LEN];
  Poker *deskPoker;
  int dispatchIndex;
  //int stage;
}Game;

int is_flush_game(Person person,Game *game);
int is_four_poker(Person person,Game *game);
int is_three_poker(Person person,Game *game);
int is_have_constant(int flag[],int value,int nums,int end,int *loc);
int fast_poker_algo(Person person,Game *game);
int is_combine_straight(int pub[],int len1,int priv[],int len2,int *max_poker);
#define __POKER_DEBUG__
#ifdef __POKER_DEBUG__
  #define LOG_DEBUG(format,...) \
   do {\
       printf("%s(line:%d),"format"\n",__func__,__LINE__,##__VA_ARGS__);\
   }while(0)
#else
  #define LOG_DEBUG(TAG,...)
#endif

#ifdef __cplusplus
}
#endif

#endif
