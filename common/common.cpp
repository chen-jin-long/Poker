#include<string.h>
#include<stdlib.h>
#include "common.h"


void dumpPrivMsg(const char *msg)
{
  Req_Poker *reqPoker = NULL;
  Req_Poker_Header head;
  reqPoker->head = &head;
  int index = 0;
  int len = strlen(msg);
  char temp[256] = {0};
  strncpy(temp,msg,sizeof(temp)-1);
  int i = 0;
  for(i = 0;i < len;i++)
  {
    if(temp[i] == '0')
    {
      temp[i] = 0;
    }
  }
  memcpy(&reqPoker->head->command,temp,sizeof(reqPoker->head->command));
  index += sizeof(reqPoker->head->command);
  memcpy(&reqPoker->head->user_id,temp+index,sizeof(reqPoker->head->user_id));
  index += sizeof(reqPoker->head->user_id);
  memcpy(&reqPoker->head->len,temp+index,sizeof(reqPoker->head->len));
  index += sizeof(reqPoker->head->len);
  printf("command : %d ",reqPoker->head->command);
  printf("user_id: %d ",reqPoker->head->user_id);
  printf("len: %d \n",reqPoker->head->len); 
  Poker *p = (Poker *)malloc(reqPoker->head->len);
  if(p == NULL) return;
  memcpy(p,temp+index,sizeof(Poker)*reqPoker->head->len); 
  for(i = 0; i < reqPoker->head->len;i++)
  {
    printf("poker: value: %d ,color = %c\n",(p+i)->value,(p+i)->color); 
  }
  free(p);

}
POKER_RETURE_PARAM get_poker(Poker *poker,char *out)
{
   if(poker == NULL || out == NULL)
   {
       return POKER_PARAM_NULL_ERROR;
   }
   snprintf(out,4,"%02d%c",poker->value,poker->color);
   return POKER_OK;

}

POKER_RETURE_PARAM get_poker_msg(Poker *poker, int num, char *out)
{
   if(poker == NULL || out == NULL)
   {
       return POKER_PARAM_NULL_ERROR;
   }
   int i = 0;
   for (i = 0; i < num; i++) {
        snprintf(out+(i*3),4,"%02d%c",(poker+i)->value,(poker+i)->color);
   }

   return POKER_OK;

}
