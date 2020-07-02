#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "Utils.h"
#include "poker.h"
#include "poker_send.h"


int trans_poker_and_color(int flag[],Poker *p){
  int value;
  char color[4] = {'s','h','c','d'};
  int k,loc;

  for(k = 0;k < ONE_UNIT_POKER;k++){
    loc = (flag[k] - 1)/13; //loc 0~3
    /*
    loc:0 ;1 2 3 .. 13
    loc:1 ;14 15 16 .. 26
    loc:2 ;27 28 .. 39
    loc:3 ;40 41 .. 52
    loc:4 ;53 54
    */
    value = flag[k] - loc*13;
    if(loc < 4)
    {
      (p+k)->value= value;
      (p+k)->color = color[loc]; 
    }
    else
    {
        (p+k)->value = 13+value;
        (p+k)->color = 's';
    }
   }
}

/* just generate 1~max ,not have zero ,otherwise int flag[] don't have zero when init*/
void srand_num(int max,int flag[]){
  int j,num;
  int zero = 1;
    for(j = 0;j < max;j++){
      num = rand()%max+zero;
      while(1){
        if(0 == is_find(flag,ONE_UNIT_POKER,num)){
          flag[j] = num;
          break;
        }else{
          num = rand()%max+zero;
        }
      }
    }
}

void create_poker(Poker *p)
{
    int flag[ONE_UNIT_POKER];
    srand(time(NULL));
    memset(flag ,0,sizeof(flag));
    srand_num(ONE_UNIT_POKER,flag);
    trans_poker_and_color(flag,p);
}


Poker * wash_poker()
{

   Poker *p = (Poker *)malloc(sizeof(Poker)*ONE_UNIT_POKER);
   if(p != NULL)
   {
      create_poker(p);
      return p;
   }
   return NULL;
}

