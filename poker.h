#include<stdio.h>

#define PRIV_LEN 2
#define PUB_LEN  5

typedef struct {
  int value;
  char color;  // 's'黑桃 ; 'h' 红心 ; 'c' 梅花 ; 'd' 方块
}Poker;

typedef struct {
 int id;
 Poker priv[PRIV_LEN];
 Poker (*best_chance)[PUB_LEN]; //最优解
}Person;

typedef struct {
 Person p[2];
 Poker (*pub)[PUB_LEN];
}Game;

int is_flush_game(Person person,Game *game);
int is_four_poker(Person person,Game *game);
int is_three_poker(Person person,Game *game);
int is_have_constant(int flag[],int value,int nums,int end,int *loc);
