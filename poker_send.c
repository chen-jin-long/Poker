#include<stdio.h>
#include<time.h>
#include "Utils.h"
#include "poker.h"

int send_all_poker(const char * file){
 
  FILE *fp = NULL;
  fp = fopen(file,"a+");
  if(fp == NULL){
    printf("not open %s\n",file);
    return 0;
  }
 int i,j,k,num;
 int flag[7]={0};
  srand(time(NULL));
 for(i = 0;i < 30;i++){
  //srand(time(NULL));
  for(j = 0;j < 7;j++){
    num = rand()%52+1;
    while(1){
      if(0 == find(flag,7,num)){
        flag[j] = num;
        break;
      }else{
        num = rand()%52+1;
      }
    }
   }
 // print_array(flag,7);
  int poker[7] = {0};
  char color[7] ={'s'};
  char color_poker[4] = {'s','h','c','d'};
  for(k = 0;k < 7;k++){
    poker[k] = flag[k] % 13;
    int loc = flag[k]/13;
    if(poker[k] == 0){
      poker[k] = 13;
      loc--;
    }
    color[k] = color_poker[loc];
    if(k == 7-1){
      printf("%d:%c",poker[k],color[k]);
      fprintf(fp,"%d:%c",poker[k],color[k]);
    }else{
      printf("%d:%c,",poker[k],color[k]);
      fprintf(fp,"%d:%c,",poker[k],color[k]);
    }
   }
    printf(";0#\n");
    fprintf(fp,";0#\n");
    memset(flag,0,7);
 }
 fclose(fp);
}
int find(int flag[],int len,int value){
  int i = 0;
  for(i = 0;i < 6;i++){
    if(flag[i] == value){
      return 1;
    }    
  }
   return 0;
}
int main(){
  send_all_poker("a.txt");
  return 0;
}

