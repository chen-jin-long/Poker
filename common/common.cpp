#include<string.h>
#include<stdlib.h>
#include "common.h"


void dumpPrivMsg(const char *msg)
{
  Req_Poker poker = {0};
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
  memcpy(&poker.command,temp,sizeof(poker.command));
  index += sizeof(poker.command);
  memcpy(&poker.user_id,temp+index,sizeof(poker.user_id));
  index += sizeof(poker.user_id);
  memcpy(&poker.len,temp+index,sizeof(poker.len));
  index += sizeof(poker.len);
  printf("command : %d ",poker.command);
  printf("user_id: %d ",poker.user_id);
  printf("len: %d \n",poker.len); 
  Poker *p = (Poker *)malloc(poker.len);
  if(p == NULL) return;
  memcpy(p,temp+index,sizeof(Poker)*poker.len); 
  for(i = 0; i < poker.len;i++)
  {
    printf("poker: value: %d ,color = %c\n",(p+i)->value,(p+i)->color); 
  }
  free(p);

}
