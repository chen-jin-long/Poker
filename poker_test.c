#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "poker.h"
#include "poker_type.h"
#include "poker_compare.h"
#define SIZE 100
/*
20 同花大顺
19 同花顺
18 四条
17 满堂红
16 同花
15 顺子
14 三条
13 两对
12 一对
11 高牌
10 普通牌

*/
int get_test_data(const char *file,Person *person,Game *game,int (*func)(Person person,Game *game),int *best){
    FILE *fp = NULL;
    int i = 0,j = 0,give_result = 0;
    fp = fopen(file,"r");
    if(fp == NULL){
        printf("can't open file.\n");
        return 0;
    }
    char buffer[SIZE];
    while(fgets(buffer,SIZE,fp) != NULL){
        printf("%s",buffer);
        char *p = buffer;
        give_result = 0;
        get_priv_data(buffer,person,game);
        get_test_result(buffer,&give_result);
        //int result = slow_test_poker_algo(*person,game);
        int result = func(*person,game);
        *best = result;
        printf("result = %d\n",result); 
        if(give_result != result){
            printf("ERROR,ERROR!!!!!\n");
            break;
        }else{
            printf("Success,Success...\n");
        } 
        if(result != 0){
          for(j = 0;j < 5;j++){
            printf("best_chance[%d] .value = %d,.color = %c\n",j,(*person->best_chance+j)->value,(*person->best_chance+j)->color);
          }
       }
       printf("\n");

    }
    fclose(fp);
    return 1;
}

void get_priv_data(const char *buffer,Person *person,Game *game){
    char *p = buffer;
    int close = 0,i = 0,count = 0,flag = 0,result = 0,num = 0,k = 0;
    char value[PRIV_LEN];
    char color;
    while(*(p+i) != '\0' && *(p+i) != '\n' && *(p+i) != ';' && (*p+i) != '#'){
        if(*(p+i) != ','){
          if(*(p+i) == ':'){
            if(count == 1){
              value[count] = '\0';
            } 
            result = atoi(value);
            i++;
            color = *(p+i);
            memset(value,0,sizeof(char)*PRIV_LEN);
          }else{
            value[count] = *(p+i);
            count ++;
          }
        }else{
          if(num < 2){
              (person->priv)[num].value = result;
              (person->priv)[num].color = color;
          }else{
              (*game->pub)[num-2].value= result;
              (*game->pub)[num-2].color = color;
          }
          num++;
          count = 0;
        }
        i++;
    }
    (*game->pub)[num-2].value= result;
    (*game->pub)[num-2].color = color;
}

void get_test_result(const char *buffer,int *result){

    char *p = buffer;
    int i = 0, begin = 0,count = 0;
    char value[3];//we just need 3 char to store poker type,and the one is '\0' to use atoi 
    while(*(p+i) != '\0' && *(p+i) != '\n' && *(p+i) != '#'){
        //printf("char is %c\n",*(p+i));
        if(*(p+i) == ';'){
            if(begin == 0){
                begin = 1;
                i++;
                continue;
            }
            else break;
        }
        if(begin == 1){
            value[count] = *(p+i);
            count ++;
        }
        if(count == 2){
            value[2] = '\0';
            break;
        }
        i++;
        //printf("===\n");
    }
    if(count <= 2)value[count] = '\0';
    else value[2] = '\0';
    *result = atoi(value);
}
int slow_algo_flush_game(int poker[],char color[]){
    int i,is_straight = 1,is_color = 1,is_royal = 0,count,det = 0;
    for(i = 0,count = 0;i<PUB_LEN-1;i++){
        if(color[i+1] - color[i] != 0){
            is_color = 0;
            break;
        }
    }
    for(i = 0;i<PUB_LEN-1;i++){
        det = poker[i+1] - poker[i];
        if(det != 1){
            if(det == (10-1) && i == 0 && poker[0] == 1)printf("maybe poker A\n");
            else{
               is_straight = 0;
               break;
            } 
        }
    }
    if(poker[1] - poker[0] == (10-1) && poker[1] == 10)is_royal = 1;
    for(i = 1;i<PUB_LEN-1;i++){
        if(poker[i+1] - poker[i] != 1){
            is_royal = 0;
        }
    }
    if(is_straight == 1 && is_color == 1){
        if(is_royal == 1)return 20;//同花大顺
        return 19;//同花顺
    }else if(is_straight == 0 && is_color == 1)return 16;//同花
    else if(is_straight == 1 && is_color == 0)return 15;//顺子
    else return 0;
}
int slow_algo_four_game(int poker[],char color){
    int i = 0,flag[PUB_LEN - 1],count = 0,loc = 0,find = 0;
    for(i = 0;i<PUB_LEN-1;i++){
        flag[i] = poker[i+1] - poker[i];
    }
    for(i=0;i< PUB_LEN-1;i++){
        //if(flag[i+1] - flag[i] == 0){
        if(flag[i] == 0){
            count ++;
        }
    }
    if(count == 3){
      for(i = 0,loc = 0;i< PUB_LEN - 1;i++){
          if(flag[i] != 0){
              loc = i;
              break;
          }
      }
      printf("loc = %d\n",loc);
      if(loc == 0 || loc == PUB_LEN -2)return 18;//四条
      else return 17; //满堂红
    }else if(count == 2){
        for(i = 0,find = 0;i< PUB_LEN - 2;i++){
            if(flag[i] == 0 && flag[i+1] == 0){
                find = 1;
                break;
            }
        }
        if(find == 1 ){
            return 14;//三条
        }else{
            //print_array(poker,PUB_LEN);
            //printf("two,two....\n");
            return 13;//两对
        } 
    }else if(count == 1){
       return 12;//一对
    }else if(count == 0){
        if(poker[0] == 1)return 11;
        else return 10;
    }else{
        return 0;
    }
 return 0;   
}
int slow_algo_poker(int pub[],char color[]){
    int type = slow_algo_flush_game(pub,color);
    if(type >= 19 )return type;
    else{
       int out = slow_algo_four_game(pub,color);
       return (type > out?type:out);
    }
}
int slow_test_poker_algo(Person person,Game *game){
    int priv[2] = {person.priv[0].value,person.priv[1].value};
    Poker pub_poker[PUB_LEN];
    int i,k,m,j;
    int result[PUB_LEN+1];
    int poker[PUB_LEN];
    int pub[PUB_LEN];
    char pub_color[PUB_LEN];
    char color[PUB_LEN];
    char best_color[PUB_LEN];
    for(i = 0;i<PUB_LEN;i++){
       pub_poker[i] = (*(game -> pub))[i]; 
       pub[i] = pub_poker[i].value;
       pub_color[i] = pub_poker[i].color;
       //printf("pub_color[%d] : %c , ",i,pub_color[i]);
    }
    //printf("\n");
    result[0] = 0;
    for(i = 0;i < PUB_LEN - 2;i++){
        for(k = i+1;k < PUB_LEN -1;k++){
            for(m = k+1;m < PUB_LEN;m++){
                poker[0] = person.priv[0].value;
                color[0] = person.priv[0].color;
                poker[1] = person.priv[1].value;
                color[1] = person.priv[1].color;
                /*
                (*person.best_chance + 0)->value = poker[0];
                (*person.best_chance + 0)->color = color[0];
                (*person.best_chance + 1)->value = poker[1];
                (*person.best_chance + 1)->color = color[1];
                */
                poker[2] = pub[i];
                color[2] = pub_color[i];
                poker[3] = pub[k];
                color[3] = pub_color[k];
                poker[4] = pub[m];  
                color[4] = pub_color[m]; 
                //在这里需要进行排序
                //printf("\n before ... \n");
                //print_array(poker,PUB_LEN);
                //print_color(color,PUB_LEN);
                //printf("===========\n");
                sort_and_color(poker,color,PUB_LEN);
                //printf("after...");
                //print_array(poker,PUB_LEN);
                //print_color(color,PUB_LEN);
                //printf("----------\n");
                int type = slow_algo_poker(poker,color);
                if(type > result[0]){
                    result[0] = type;
                    for(j = 0;j<PUB_LEN;j++){
                       result[j+1] = poker[j];
                       best_color[j] = color[j];
                    }
                }else if(type == result[0]){
                    //这里需要比较大小的算法，即相同类型的牌时需要比较谁更大
                    //if(0 != compare_poker_algo(poker,result+1,type)){ // bug,not think A 1;
                    //printf("CGD type = %d\n",type);
                    if(1 == compare_poker_algo(poker,result+1,type)){ // bug,not think A 1;
                        result[0] = type;
                        for(j = 0;j<PUB_LEN;j++){
                            result[j+1] = poker[j];
                            best_color[j] = color[j];
                        }
                    }
                    //printf("the same\n");
                }else{
                    //printf("lower\n");
                }
            }
        }
    }
    for(i = 0;i< PUB_LEN;i++){
       (*person.best_chance + i)->value = result[i+1];
       (*person.best_chance + i)->color = best_color[i];
    }
    return result[0];
}
int main(){
    Poker max_chance[5] = {{0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'s'}};
    Person p1 = { 0,{{0,'s'},{0,'s'}} ,&max_chance};
    Poker pub[5] = { {0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'d'} };
    Game game = {{p1,p1},&pub};
    int best = 0;
    get_test_data("data.txt",&p1,&game,slow_test_poker_algo,&best);
    //int test_result = 0;
   // get_test_result(";",&test_result);
    //printf("reselt: %d\n",test_result);
    /*
    for(i = 0;i<PRIV_LEN;i++)
       printf("priv[%d]:value = %d,color = %c\n",i,p1.priv[i].value,p1.priv[i].color);
    }   
    for(i=0;i<PUB_LEN;i++){
        printf("pub[%d]:value = %d,color = %c\n",i,(*game.pub)[i].value,(*game.pub)[i].color);
    }
    */
    /*
    Poker max_chance[5] = {{0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'s'}};
    Person p1 = { 0,{{0,'s'},{0,'s'}} ,&max_chance};
    Poker pub[5] = { {0,'s'},{0,'s'},{0,'s'},{0,'s'},{0,'d'} };
    Game game = {{p1,p1},&pub};
    */
    return 0;
}
