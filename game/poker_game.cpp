#include<stdio.h>
#include<stdlib.h>
#include "poker_game.h"

void initDeskStage(POKER_DESK *desk);

int setupPokerRoom(POKER_ROOM *pr)
{
  int i = 0;
  for(i = 0;i < MAX_POKER_ROOM;i++)
  {
     pr->room_id = i;

  }
  for(i = 0;i < MAX_POKER_DESK;i++)
  {
     (pr->pdesk)[i] = NULL;
  }

}

POKER_DESK * setupPokerDesk(int desk_id,POKER_ROOM *proom)
{
  if((proom->pdesk)[desk_id] == NULL)
  {
     printf("desk for %d was not setuped,now we will malloc\n",desk_id);
     (proom->pdesk)[desk_id] =(POKER_DESK *) malloc(sizeof(POKER_DESK));
     (proom->pdesk)[desk_id]->desk_id = desk_id;
     initDeskStage((proom->pdesk)[desk_id]);

  }
  else
  {
     printf("desk for %d was already setuped\n",desk_id);
  }


  return (proom->pdesk)[desk_id];
}

void initDeskStage(POKER_DESK *desk)
{
  desk->stage = 0;

}
