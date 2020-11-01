#ifndef _POKER_GAME_H
#define _POKER_GAME_H
#include "poker.h"

#define MAX_POKER_DESK 3
#define MAX_DESK_PLAYER 3
#define MAX_POKER_ROOM 1
#define POKER_GAME_INIT_MONEY 10

typedef struct{
  int desk_id;
  //char *player[MAX_DESK_PLAYER];
  Person person[MAX_DESK_PLAYER];
  int stage;
  int betMoney;
  Game *game;
  int winer;
  //POKER_PUBLIC
}POKER_DESK;

typedef struct{
  int room_id;
  POKER_DESK *pdesk[MAX_POKER_DESK];

}POKER_ROOM;

typedef struct WinerTag{
    int index;
    int score;
    struct WinerTag *next;
}Winer;

int setupPokerRoom(POKER_ROOM *pr);

POKER_DESK * setupPokerDesk(int desk_id,POKER_ROOM *proom);
void InitGamePubPoker(Game *game, Poker (*pub)[PUB_LEN]);
Winer * getGameWiner(Person *person);
void printWiner(Winer *head);
void freeAllWiner(Winer *head);

#endif
