#include<stdio.h>
#include<stdlib.h>
#include "poker.h"
#include "Utils.h"

int main(){
  //Person p1 = { 10,{{2,'s'},{3,'s'}} };
  Poker max_chance_1[5] = {{0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'s'}};
  Poker max_chance_2[5] = {{0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'s'}};
  Person p1 = { 10,{{8,'s'},{12,'s'}} ,&max_chance_1};
  //Person p2 = { 11,{{3,'c'},{5,'d'}} ,&max_chance_2};
  // Person p2 = { 11,{{3,'s'},{5,'s'}} ,&max_chance_2};
  Person p2 = { 11,{{5,'s'},{4,'r'}} ,&max_chance_2};
  //Poker pub[5] = { {7,'s'},{8,'s'},{9,'s'},{10,'s'},{13,'d'} };
  Poker pub[5] = { {6,'s'},{2,'a'},{4,'s'},{3,'s'},{7,'d'} };
  //Poker pub[5] = { {1,'s'},{9,'s'},{10,'s'},{11,'s'},{13,'d'} };
  Game game;
  //game.p[0] = p1;
  //game.p[1] = p2;
  game.pub = &pub;
  int weight1 = 0,weight2 = 0,j = 0;
  //int result = is_flush_poker(p2,&game);
  //int result = fast_poker_algo(p2,&game);
  //printf("result = %d\n",result);
  //printf("p1 result = %d,weight = %d\n", is_flush_game(p1,&game,&weight1),weight1);
  //printf("p2 result = %d,weight = %d\n", is_flush_game(p2,&game,&weight2),weight2);
  //printf("weight1 = %d,weight2 = %d\n",weight1,weight2);
  sort_poker_game(&p2,&game);
  printf("value = %d,color = %c \n",p2.priv[0].value,p2.priv[0].color);
  printf("value = %d,color = %c \n",p2.priv[1].value,p2.priv[1].color);
  printf("value = %d,color = %c \n",pub[0].value,pub[0].color);
  /*
  for(j = 0;j < 5;j++){
   printf("best_chance[%d] .value = %d,.color = %c\n",j,(*p2.best_chance+j)->value,(*p2.best_chance+j)->color);
  }
  */
  //printf("============next==========\n");
  //is_three_poker(p2,&game);
  //is_two_poker(p2,&game);
  //is_four_poker(p2,&game);
  //for(j=0;j<5;j++){
   //printf("best_chance[%d] .value = %d,.color = %c,max_chance.value = %d\n",j,(*p2.best_chance+j)->value,(*p2.best_chance+j)->color,max_chance_2[j].value);
 //    printf("%p,%p,%c\n",p1.best_chance,(*p1.best_chance+j),(*p1.best_chance+j)->color);
  //}
//  printf("sizeof Poker = %d\n",(int)sizeof(Poker));
 // printf("is_four_poker,reulst = %d\n",is_four_poker(p2,&game));
  printf("==========end==============\n");
  return 0;
}
