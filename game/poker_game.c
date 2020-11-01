#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "poker_game.h"
#include "common.h"

#define DO_COMPARE_WITH_SUCCESS 1
#define NOT_COMPARE_WITH_FAIL 0
void initDeskStage(POKER_DESK *desk);
void freeAllWiner(Winer *head);

int setupPokerRoom(POKER_ROOM *pr)
{
  int roomId = 0;
  int deskId = 0;
  for(roomId = 0;roomId < MAX_POKER_ROOM;roomId++)
  {
      pr[roomId].room_id = roomId;
      for(deskId = 0;deskId < MAX_POKER_DESK;deskId++)
      {
         //(pr[roomId].pdesk)[deskId] = setupPokerDesk(deskId, pr);
         setupPokerDesk(deskId, &pr[roomId]);
      }
  }
  return 0;
}

POKER_DESK * setupPokerDesk(int desk_id,POKER_ROOM *proom)
{
  POKER_DESK *desk = NULL;
  int id = 0;
  if((proom->pdesk)[desk_id] == NULL)
  {
     printf("desk for %d was not setuped,now we will malloc\n",desk_id);
     desk = (POKER_DESK *) malloc(sizeof(POKER_DESK));
     if (desk == NULL) {
          return NULL;
     }
     (proom->pdesk)[desk_id] = desk;
     (proom->pdesk)[desk_id]->desk_id = desk_id;
     initDeskStage((proom->pdesk)[desk_id]);
     proom->pdesk[desk_id]->game = (Game *)malloc(sizeof(Game));
      for (id = 0; id < MAX_DESK_PLAYER; id++) {
        proom->pdesk[desk_id]->person[id].id = 0;
        //proom->pdesk[desk_id]->person[id].priv = 
        proom->pdesk[desk_id]->person[id].status = POKER_ACTION_LOGIN;
        proom->pdesk[desk_id]->person[id].best_chance = (Poker (*)[PUB_LEN])malloc(sizeof(Poker)*PUB_LEN);
      }
     //proom->pdesk[desk_id]->game->pub = &g_game_pub;
  }
  else
  {
     printf("desk for %d was already setuped\n",desk_id);
  }
  return (proom->pdesk)[desk_id];
}

void initDeskStage(POKER_DESK *desk)
{
  desk->stage = POKER_STAGE_LOGIN;
  desk->betMoney = POKER_GAME_INIT_MONEY;
}

void InitGamePubPoker(Game *game, Poker (*pub)[PUB_LEN])
{
     if (game != NULL) {
       game->pub = pub;
     }
}

/*
    attender[i][0] = connId;
    attender[i][1] = score;
*/
void sortArray(int attender[][2], int len) {
    int i, j, curScore,curConnId,index;
    for (i = 0,j = 0; i < len; i++) {
        curScore = attender[i][1];
        curConnId = attender[i][0];
        index = i; 
        for (j = i-1; j >= 0; j--) {
            if (curScore > attender[j][1]) {
                attender[index][1] = attender[j][1];
                attender[index][0] = attender[j][0];
                index = j;
                attender[j][1] = curScore;
                attender[j][0] = curConnId; 
            } else {
                break;
            }
        }
    }
}


typedef struct {
    int *flag;
    int num;
    int index;
    int compare;
}GameFlagRest;

typedef Poker (*SORTED_BEST)[PUB_LEN];
typedef int * (*GetFlagScoreFunc)(SORTED_BEST best, int *num);
typedef Winer * (*CompareGameScoreByFlag)(int level, GameFlagRest *scoreFlag, int count);

typedef struct {
    int scoreLevel;
    GetFlagScoreFunc func;
    CompareGameScoreByFlag compare;
}GameScoreFlag;

#define THREE_KIND_FLAG_NUM 3

/*
#define POKER_TYPE_MAX_FLUSH_FLUSH 20
#define POKER_TYPE_FLUSH_STRAIGHT 19
#define POKER_TYPE_FOUR 18
#define POKER_TYPE_FULLHOUSE 17
#define POKER_TYPE_FLUSH 16
#define POKER_TYPE_STRAIGHT 15
#define POKER_TYPE_THREE_KIND 14
#define POKER_TYPE_TWO_PAIR 13
#define POKER_TYPE_ONE_PAIR 12
#define POKER_TYPE_A 11
#define POKER_TYPE_SINGLE 10 
#define POKER_TYPE_UNKNOW 0


*/
int * GetFlagScore_FlushSgt(Poker (*sortedBestChance)[PUB_LEN], int *num)
{
    if (num == NULL) {
        return NULL;
    }
    int *flag = (int *)malloc(sizeof(int));
    if (flag != NULL) {
        *flag = (*sortedBestChance)[PUB_LEN-1].value;
           *num = 1;
    } else {
        *num = 0;
    }
    return flag;
}

int getThreeSamePoker(SORTED_BEST best)
{
    int i = 0;
    int num = 0;
    for (i = 0; i < PUB_LEN -1; i++) {
        if ((*best + i)->value == (*best + i + 1)->value) {
            num++;
            if (num == 2) {
                return (*best + i)->value;
            }
        }
    }
    return -1;
}

int * GetContiThreeFlagScore(SORTED_BEST best, int *num)
{
    if (num == NULL) {
        return NULL;
    }
    int result = getThreeSamePoker(best);
    if (result <= 0) {
        *num = 0;
        return NULL;
    }

    int *flag = (int *)malloc(sizeof(int)*THREE_KIND_FLAG_NUM);
    if (flag == NULL) {
        return flag;
    }
    // 第一个需要比较的标志: 相同的三个牌
   *(flag + 0) = result;
   *num = 1;
    int i = 0;
    for (i = 0; i < PUB_LEN; i++){
        if (result != (*best + i)->value) {
            *(flag + (*num)) = (*best + i)->value;
            (*num)++;
        }
    }
    if (*num != THREE_KIND_FLAG_NUM) {
        printf("[%s] num =%d , is not matched.\n", __FUNCTION__, *num);
        free(flag);
        flag = NULL;
    }
    return flag;
}

int getMax(int *elements, int num)
{
    int i = 0;
    int max = elements[0];
    for (i = 0; i < num; i++) {
        if(elements[i] >= max) {
            max = elements[i];
        }
    }
    return max;
}

Winer * CompareOneFlagScore(int level, GameFlagRest *scoreFlag, int count)
{
    int i = 0, max = 0, index = 0;
    for (i = 0; i < count; i++) {
       if (scoreFlag && scoreFlag[i].num == 1) {
            if ( i == 0) {
                max = scoreFlag[0].flag[0];
                index = 0;
            } else {
                if (scoreFlag[i].flag[0] >= max) {
                    max = scoreFlag[i].flag[0];
                    index = i;
                }
            }
       }
    }
    Winer *win = (Winer *)malloc(sizeof(Winer));
    if (win != NULL) {
        win->index = index;
        win->score = max;
        win->next = NULL;
    }
    return win;
}

Winer * CompareFlagScore(int level, GameFlagRest *scoreFlag, int count)
{
    int i = 0, j = 0, max = -1;
    Winer *win = NULL;
    Winer *head = NULL;
    Winer *cur = NULL;

    if (scoreFlag == NULL) {
        return NULL;
    }
    /*
        依次比较1,2,3,..., 并通过compare标志位将小数淘汰，下一轮不再比较。
        最终剩下的就是获胜者，有可能存在多个。
        注意：由于是同类比较，每个参赛者的flag个数是相同的。
        A1 A2 A3 A4
        B1 B2 B3 B4
        C1 C2 C3 C4
        D1 D2 D3 D4
    */
    for (j = 0; j < scoreFlag[0].num; j++) {
        for (i = 0, max = -1; i < count; i++) {
            if (scoreFlag[i].compare == DO_COMPARE_WITH_SUCCESS) {
                if (scoreFlag[i].flag[j] < max) {
                    scoreFlag[i].compare = NOT_COMPARE_WITH_FAIL;
                } else {
                    max = scoreFlag[i].flag[j];
                }
            }
       }
    }

    for(i = 0; i < count; i++) {
        if (scoreFlag[i].compare == DO_COMPARE_WITH_SUCCESS) {
            win = (Winer *)malloc(sizeof(Winer));
            if (win == NULL) {
                 for (j = 0; j < 5; j++) {
                    win = (Winer *)malloc(sizeof(Winer));
                    if (win) {
                       break;
                    }
                 }
            }

            if (win) {
                win->index = i;
                win->score = level;
                win->next = NULL;
                if (head == NULL) {
                    head = win;
                    cur = head;
                } else {
                    cur->next = win;
                }
            } else {
                freeAllWiner(head);
                return NULL;
            }
        }
    }
    return head;
}


GameScoreFlag gameFlag[] = {
    {POKER_TYPE_FLUSH_STRAIGHT, GetFlagScore_FlushSgt, CompareFlagScore},
    {POKER_TYPE_FOUR, GetContiThreeFlagScore, CompareFlagScore},
    {POKER_TYPE_FULLHOUSE, GetContiThreeFlagScore, CompareFlagScore},
    {POKER_TYPE_THREE_KIND, GetContiThreeFlagScore, CompareFlagScore}
};


int * getFlagScore(int level, SORTED_BEST best, int *num)
{
    int i = 0;
    int *result = NULL;
    for (i = 0; i < sizeof(gameFlag)/sizeof(GameScoreFlag); i++) {
        if (gameFlag[i].scoreLevel == level) {
            result = gameFlag[i].func(best, num);
            return result;
        }
    }
    return result;
}

Winer * compareFlagScore(int level, GameFlagRest *scoreFlag, int indexCount)
{
    int i = 0;
    Winer * result = NULL;
    for (i = 0; i < sizeof(gameFlag)/sizeof(GameScoreFlag); i++) {
        if (gameFlag[i].scoreLevel == level) {
            result = gameFlag[i].compare(level, scoreFlag, indexCount);
            return result;
        }
    }
    return result;
}

Winer * getComplxGameWiner(Person *person)
{
    int i = 0, index = 0, num = 0;
    GameFlagRest *scoreFlag = (GameFlagRest *)malloc(sizeof(GameFlagRest) * PUB_LEN);
    if (scoreFlag == NULL) {
        return NULL;
    }
    memset(scoreFlag, 0, sizeof(GameFlagRest) * PUB_LEN);
    int score = person[0].gameScore;
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
       if( person[i].gameScore == score) {
            scoreFlag[index].flag = getFlagScore(person[i].gameScore, person[i].best_chance, &num);
            scoreFlag[index].num = num;
            scoreFlag[index].index = i;
            scoreFlag[index].compare = DO_COMPARE_WITH_SUCCESS;
            index++;
       }
    }
    return compareFlagScore(score, scoreFlag, index);
}

Winer * getGameWiner(Person *person)
{
    if (person == NULL) {
        return NULL;
    }

    int i = 0;
    int attender[MAX_DESK_PLAYER][2];
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        attender[i][0]= i;
        attender[i][1] = person[i].gameScore;
    }
    Winer *head = NULL;

    sortArray(&attender[0], MAX_DESK_PLAYER);
    for (i = 0; i < MAX_DESK_PLAYER; i++) {
        if (attender[0][1] > attender[1][1]) {
            head = (Winer *)malloc(sizeof(Winer));
            if (head == NULL) {
                printf("mallck Winer failed..\n");
                return NULL;
            }
            head->index = attender[0][0];
            head->score = attender[0][1];
            head->next = NULL;
            return head;
        } else if (attender[0][1] == attender[1][1]) {
            printf("[%s] equal result:%d\n", __FUNCTION__, attender[0][1]);
            head = getComplxGameWiner(person);
            return head;
        } else {
            printf("[%s]error sort\n", __FUNCTION__);
            return head;
        }
    }

    return NULL;
}

void printWiner(Winer *head)
{
    Winer *cur = head;
    while(cur){
         printf("====winer connId:%d, Score:%d===\n", cur->index, cur->score);
         cur = cur->next;
    }
}

void freeAllWiner(Winer *head)
{
    Winer *cur = head;
    while (head) {
        free(cur);
        cur = NULL;
        head = head->next;
    }
}

