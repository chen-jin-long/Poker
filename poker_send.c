#include<stdio.h>
#include<time.h>
#include "Utils.h"
#include "poker.h"
typedef int (* SEND)(FILE *fp,int rounds);
#define ONE_POKER 7

int store_poker_file(FILE *fp,int poker[],char color[]){
  int k;
  for(k = 0;k < 7;k++){
    if(k == 7-1){
      //printf("%d:%c",poker[k],color[k]);
      fprintf(fp,"%d:%c",poker[k],color[k]);
    }else{
      //printf("%d:%c,",poker[k],color[k]);
      fprintf(fp,"%d:%c,",poker[k],color[k]);
    }
  }
 // printf(";0#\n");
  fprintf(fp,";0#\n");
}

int trans_poker_and_color(int flag[],int poker[],char color[]){
  int temp_poker[7] = {0};
  //char temp_color[7] ={'s'};
  char color_poker[4] = {'s','h','c','d'};
  int k;
  for(k = 0;k < 7;k++){
   temp_poker[k] = flag[k] % 13;
    int loc = flag[k]/13;
    if(temp_poker[k] == 0){
      temp_poker[k] = 13;
      loc--;
    }
    //temp_color[k] = color_poker[loc];
    poker[k] = temp_poker[k];
    color[k] = color_poker[loc]; 
   }
}

/* just generate 1~max ,not have zero ,otherwise int flag[] don't have zeor when init*/
void srand_num(int max,int flag[],int zero){
  int j,num;
  zero = 1;
    for(j = 0;j < 7;j++){
      num = rand()%max+zero;
      while(1){
        if(0 == is_find(flag,7,num)){
          flag[j] = num;
          break;
        }else{
          num = rand()%max+zero;
        }
      }
    }
}
int send_all_poker(int type,FILE *fp,int rounds){
   int i,flag[7],poker[7];
   char color[7];
   for(i = 0;i < rounds;i++){
     memset(flag,0,7*sizeof(int));
     srand_num(52,flag,1);
     trans_poker_and_color(flag,poker,color);
     store_poker_file(fp,poker,color);
    // memset(flag,0,7);
   }
}
/*
int find(int flag[],int len,int value){
  int i = 0;
  for(i = 0;i < len-1;i++){
    if(flag[i] == value){
      return 1;
    }    
  }
   return 0;
}
*/

int send_special_poker(FILE *fp,int rounds){
    //int base[7] = {2,2,2,2,1,5,7};
    //int base[7] = {2,2,2,3,3,4,4};
    //int base[7] = {1,2,3,4,5,6,7};
    int base[7] = {1,8,9,10,11,12,13};
    int flag[7];
    int poker[7];
    char color[7];
    memset(flag,0,7*sizeof(int));
    memset(color,'s',7*sizeof(char));
    int j,i;
   for(i = 0;i < rounds;i++){
     memset(flag,0,7*sizeof(int));
     srand_num(7,flag,1);
     for(j = 0;j < 7;j++){
       poker[j] = base[flag[j]-1];
     }
    store_poker_file(fp,poker,color);
   }
}

int send_poker(const char * file,SEND send){
  FILE *fp = NULL;
  fp = fopen(file,"a+");
  if(fp == NULL){
    printf("not open %s\n",file);
    return 0;
  }
  int rounds = 30;
  send(fp,rounds);
  if(fp != NULL)fclose(fp);

}
int main(int argc,char *argv[]){
  if(argc < 2){
    printf("======help==========\n");
    printf("please input ./send.out type filename,eg. ./send.out 0/1\n");
    return 0; 
  }
  int type = atoi(argv[1]);
  srand(time(NULL));
  if(type == 0){
    send_poker("all.txt",send_all_poker);
  }else if(type == 1){
    send_poker("special.txt",send_special_poker);
  }
  //send_poker("four.txt",send_four_poker);
  return 0;
}

