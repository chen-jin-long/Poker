#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "Utils.h"
#include "poker.h"
#include "poker_send.h"
#include "poker_game.h"

#define FIX_POKER_FILE "fix_poker.txt"

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

Poker *get_fixed_poker(const char *buffer)
{
    int i = 0, count = 0, result = 0, num = 0;
    char value[PRIV_LEN];
    char color;
    int len = (PUB_LEN + PRIV_LEN*MAX_DESK_PLAYER);
    Poker *pokerMsg = (Poker *)malloc(sizeof(Poker) * len);
    if (pokerMsg == NULL) {
        return NULL;
    }
    memset(pokerMsg, 0, len);
    while(*(buffer+i) != '\0' && *(buffer+i) != '\n' && *(buffer+i) != ';' && *(buffer+i) != '#'){
        if(*(buffer+i) != ','){
          if(*(buffer+i) == ':'){
            if(count == 1){
              value[count] = '\0';
            } 
            result = atoi(value);
            i++;
            color = *(buffer+i);
            memset(value, 0, sizeof(char)*PRIV_LEN);
          }else{
            value[count] = *(buffer+i);
            count ++;
          }
        }else{
            pokerMsg[num].value = result;
            pokerMsg[num].color = color;
          num++;
          count = 0;
        }
        i++;
    }
    pokerMsg[num].value= result;
    pokerMsg[num].color = color;
    return pokerMsg;
}

Poker * get_fixed_poker_from_file(const char * name)
{
    FILE *fp = fopen(name, "r");
    char buf[64] = {0};
    Poker * pk = NULL;
    if (fp) {
        if (fgets(buf, sizeof(buf), fp) != NULL) {
            pk = get_fixed_poker(buf);
        }
        fclose(fp);
    } else {
        printf("[%s] can't open %s\n", __FUNCTION__, name);
    }
    return pk;
}
Poker * wash_poker()
{
    Poker *p = NULL;
#ifdef FIXED_POKER
    printf("start fixed poker game..\n");
    p = get_fixed_poker_from_file(FIX_POKER_FILE);
    if (p == NULL) {
        printf("get_fixed_poker error!\n");
    }
#endif

    if (p == NULL) {
        printf("start srand poker game..\n");
       p = (Poker *)malloc(sizeof(Poker)*ONE_UNIT_DK_POKER);
       if(p != NULL)
       {
          create_poker(p);
          return p;
       }
    }
    return p;
}

