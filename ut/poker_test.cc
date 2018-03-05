#include <iostream>
#include <limits.h>
#include <time.h>
#include "gtest/gtest.h"
#include "poker.h"
namespace {
using namespace std;
class PokerTest : public testing::Test {
 protected:
  virtual void SetUp() {
    int i;
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
  }

  virtual void TearDown() {
    const time_t end_time = time(NULL);
    cout << "Time: " << end_time - start_time << endl;
  }

  time_t start_time;
  Poker max_chance[5];
  Poker pub[5];
  Person p1;
  Person p2;
  Game game;
};
 
  TEST_F(PokerTest,is_combine_straight_test){
     int pub[5] = {1,2,3,4,5};
     int priv[2] = {1,2};
     int max = 0;
     EXPECT_EQ(1,is_combine_straight(pub,5,priv,2,&max));
  }

  TEST_F(PokerTest,DefaultConstructor) {
    EXPECT_EQ(10,p1.id);
  }
}

int main(int argc,char *argv[]){
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
