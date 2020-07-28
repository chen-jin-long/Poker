#include<stdio.h>
#include<stdlib.h>
#include "poker_game.h"
#include "common.h"

void initDeskStage(POKER_DESK *desk);

int setupPokerRoom(POKER_ROOM *pr)
{
  int roomId = 0;
  int deskId = 0;
  for(roomId = 0;roomId < MAX_POKER_ROOM;roomId++)
  {
      pr[roomId].room_id = roomId;
      for(deskId = 0;deskId < MAX_POKER_DESK;deskId++)
      {
         //(pr[roomId].pdesk)[deskId] = setupPokerDesk(deskId, pr);
         setupPokerDesk(deskId, &pr[roomId]);
      }
  }
  return 0;
}

POKER_DESK * setupPokerDesk(int desk_id,POKER_ROOM *proom)
{
  POKER_DESK *desk = NULL;
  if((proom->pdesk)[desk_id] == NULL)
  {
     printf("desk for %d was not setuped,now we will malloc\n",desk_id);
     desk = (POKER_DESK *) malloc(sizeof(POKER_DESK));
     if (desk == NULL) {
          return NULL;
     }
     (proom->pdesk)[desk_id] = desk;
     (proom->pdesk)[desk_id]->desk_id = desk_id;
     initDeskStage((proom->pdesk)[desk_id]);
     proom->pdesk[desk_id]->game = (Game *)malloc(sizeof(Game));
     //proom->pdesk[desk_id]->game->pub = &g_game_pub;
  }
  else
  {
     printf("desk for %d was already setuped\n",desk_id);
  }
  return (proom->pdesk)[desk_id];
}

void initDeskStage(POKER_DESK *desk)
{
  desk->stage = POKER_STAGE_LOGIN;
  desk->betMoney = POKER_GAME_INIT_MONEY;
}

void InitGamePubPoker(Game *game, Poker (*pub)[PUB_LEN])
{
     if (game != NULL) {
       game->pub = pub;
     }
}