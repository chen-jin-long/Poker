#include <iostream>
#include <limits.h>
#include <time.h>
#include "gtest/gtest.h"
#include "poker.h"
#include "Utils.h"
#include "poker_send.h"

namespace {
using namespace std;
class PokerTest : public testing::Test {
 protected:
  virtual void SetUp() {
    int i = 0;
    for(i = 0;i < 5;i++)
      max_chance[i] = {0,'s'};
    p1 = {10,{{8,'s'},{12,'s'}},&max_chance};
    p2 = {11,{{5,'s'},{4,'r'}},&max_chance};
    pub[0]  = {6,'s'};
    pub[1]  = {2,'a'};
    pub[2]  = {4,'s'};
    pub[3]  = {3,'s'};
    pub[4]  = {7,'d'};
    game.p[0] = p1;
    game.p[1] = p2;
    game.pub = &pub;
    char color[4] = {'s','h','c','d'};   
    p = (Poker *)malloc(sizeof(Poker)*ONE_UNIT_POKER);
    for(i = 0; i<ONE_UNIT_POKER-2;i++)
    {
      (p+i)->value = i%13 + 1;
      (p+i)->color = color[i/13]; 
      // 0 1 2 3 4 5 .. 12
      //13 14 15 16....25
    }
    (p+ONE_UNIT_POKER-2)->value = 14;
    (p+ONE_UNIT_POKER-2)->color = 's';
    (p+ONE_UNIT_POKER-1)->value = 15;
    (p+ONE_UNIT_POKER-1)->color = 's';
  }

  virtual void TearDown() {
    const time_t end_time = time(NULL);
    cout << "Time: " << end_time - start_time << endl;
    if(p != NULL)free(p);
  }

  time_t start_time;
  Poker max_chance[5];
  Poker pub[5];
  Person p1;
  Person p2;
  Game game;
  Poker *p = NULL;
};
 
  TEST_F(PokerTest,is_combine_straight_test){
     int pub[5] = {9,10,11,12,13};
     int priv[2] = {8,9};
     int max = 0,i = 0;
     for(i = 1;i < 5;i++){
       priv[1] = 8 + i; 
       EXPECT_EQ(1,is_combine_straight(pub,5,priv,2,&max));
       EXPECT_EQ(12,max);
     }

     //7,8,9,10,11
     priv[0] = 7;
     priv[1] = 8;
     EXPECT_EQ(1,is_combine_straight(pub,5,priv,2,&max));
     EXPECT_EQ(11,max);

     //test 10,11,12,13,1
     priv[0] = 1;
     for(i = 0;i < 4;i++){
       priv[1] = 10 + i;
       EXPECT_EQ(1,is_combine_straight(pub,5,priv,2,&max));
       EXPECT_EQ(14,max);
     }
  }
  
  TEST_F(PokerTest,is_four_poker) {

    Poker max_chance[5] = {{0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'s'}};
    Person p1 = { 0,{{9,'s'},{10,'s'}} ,&max_chance};
    Poker pub[5] = { {9,'s'},{9,'s'},{9,'s'},{10,'s'},{10,'s'} };
    Game game = {{p1,p1},&pub};
    EXPECT_EQ(POKER_TYPE_FOUR,is_four_poker(p1,&game));
    int i = 0;
    for(i = 0;i < 4;i++){
      //EXPECT_EQ(9,(*(p1.best_chance))[i].value);
      EXPECT_EQ(9,(*p1.best_chance)[i].value);
    }
      EXPECT_EQ(10,(*p1.best_chance)[4].value);
  }

  TEST_F(PokerTest,DefaultConstructor) {
    EXPECT_EQ(10,p1.id);
  }
 
  TEST_F(PokerTest,get_poker)
  {
    Poker *find = NULL;
    int i = 0;
    find = get_poker(); 
    if(p != NULL && find != NULL)
    {
      for(i = 0;i<ONE_UNIT_POKER;i++)
      {
        EXPECT_EQ(1,find_poker(p,find+i));
      }
      free(find);
    }

  }
}

int main(int argc,char *argv[]){
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}