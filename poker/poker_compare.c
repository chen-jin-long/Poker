#include<stdio.h>
#include "poker.h"
#include "Utils.h"
#include "poker_type.h"
int compare_straight(int pk1[],int pk2[]){
     int i = 0;
     for(i = PUB_LEN -1;i >= 0;i--){
         if(pk1[i] != pk2[i]){  //bug ,not think poker A;
            if(i == 0){
                if(pk1[i] == 1) return 1; // 只需要考虑 8 9 10 11 12，和 1 9 10 11 12即可
                else if(pk2[i] == 1) return 2;
            }
            return pk1[i] > pk2[i] ? 1:2;
         }
     }
     return 0;
}

int compare_four(int pk1[],int pk2[]){
    int i = 0,flag_one[PUB_LEN-1],flag_two[PUB_LEN];
    int value_one[2],value_two[2];
    for(i = 0;i < PUB_LEN -1 ;i++){
        flag_one[i] = pk1[i+1] - pk1[i];
        flag_two[i] = pk2[i+1] - pk2[i];
    }
    for(i = 0;i < PUB_LEN;i++){
        if(flag_one[i] == 0){
            value_one[0] = pk1[i];
        }else{
            value_one[1] = pk2[i];
        }
    }
    for(i = 0;i < PUB_LEN-1;i++){
        if(flag_two[i] == 0){
            value_two[0] = pk1[i];
        }else{
            value_two[1] = pk2[i];
        }
    }
    if(value_one[0] > value_two[0] || (value_one[0] == value_two[0] && value_one[1] > value_two[1])){
          return 1;
    }else if(value_one[0] < value_two[0] || (value_one[0] == value_one[1] && value_one[1] < value_two[1])){
          return 2;
    }else{
        return 0;
    }
}
int compare_value(int val1,int val2){
    if(val1 == val2)return 0;
    else{
        if(val1 > val2){
            if(val2 == 1)return 2;
            else return 1;
        }else{
            if(val1 == 1)return 1;
            else return 2;
        } 
    }
    return -1;
}

int compare_three(int poker1[],int poker2[],int type){
    int value1[3] = {0,0,0};
    int value2[3] = {0,0,0};
    int i;
    int type1 = 0,type2 = 0;
    sort(poker1,PUB_LEN);
    sort(poker2,PUB_LEN);
    get_three_poker_val(poker1,value1);
    get_three_poker_val(poker2,value2);
    //print_array(value1,3);
    //print_array(value2,3);
      if(value1[0] == value2[0]){
        if(type  == POKER_TYPE_FULLHOUSE){
            return compare_value(value1[1],value2[1]);
        }else if(type == POKER_TYPE_THREE_KIND){
            if(value1[2]  == value2[2]){
                return compare_value(value1[1],value2[1]);
            }else return compare_value(value1[2],value2[2]);
        }else{
          printf("type err\n");
        }
      }else{
         return compare_value(value1[0],value2[0]);
      }
    return 0;
}
int compare_two(int poker1[],int poker2[],int type){
    int value1[4] = {0,0,0,0};
    int value2[4]= {0,0,0,0};
    int flag1[4];
    int flag2[4];
    int i,k,count;
    for(i = 0;i<PUB_LEN-1;i++){
        flag1[i] = poker1[i+1]-poker1[i];
        flag2[i] = poker2[i+1]-poker2[i];
    }
    for(i = 0,count = 0,k = 3;i < PUB_LEN-1;i++){
        if(flag1[i] == 0){
            value1[count] = poker1[i];
            if(poker1[i] == 1)value1[count] = 14;
            count ++;
        }else{
            value1[k] = poker1[i];
            if(poker1[i] == 1)value1[k] = 14;
            k--;
        }
    }
    for(i = 0,count = 0,k = 3;i < PUB_LEN-1;i++){
        if(flag2[i] == 0){
            value2[count] = poker2[i];
            if(poker2[i] == 1)value2[count] = 14;
            count ++;
        }else{
            value2[k] = poker2[i];
            if(poker2[i] == 1)value2[k] = 14;
            k--;
        }
    }
   if(type == POKER_TYPE_TWO_PAIR){
       sort(value1,2);
       sort(value2,2);
       if(value1[1] == value2[1]){
          int res = compare_value(value1[0],value2[0]);//solve bug
          if(res == 0)return compare_value(value1[3],value2[3]);
          else return res;
       }else{
          return compare_value(value1[1],value2[1]);
       }
   }else if(type == POKER_TYPE_ONE_PAIR){
      sort(value1+1,3);
      sort(value2+3,3);
      if(value1[0] == value2[0]){
         for(i = 3;i > 0;i--){
             if(value1[i] != value2[i])return compare_value(value1[i],value2[i]);
         }
         return 0;
      }else{
          return compare_value(value1[0],value2[0]);
      }
   }    
}
int compare_array(int poker1[],int poker2[],int len){
    int i = 0;
    for(i = len-1;i >= 0;i--){
        if(poker1[i] != poker2[i])return compare_value(poker1[i],poker2[i]);
    }
    return 0;
}
int compare_one(int poker1[],int poker2[],int type){
    if(type == POKER_TYPE_A){
        return compare_array(poker1+1,poker2+1,PUB_LEN-1);
    }else if(type == POKER_TYPE_SINGLE){
        return compare_array(poker1,poker2,PUB_LEN);
    }
    return -1;
}  
int compare_flush(int poker1[],int poker2[]){
    int i = 0;
    int poker1_temp[PUB_LEN];
    int poker2_temp[PUB_LEN];
    for(i = 0;i < PUB_LEN;i++ ){
        poker1_temp[i] = poker1[i];
        poker2_temp[i] = poker2[i];
        if(poker1[i] == 1)poker1_temp[i] = 14; // you must note this code,because you change 1 to 14;
        if(poker2[i] == 1)poker2_temp[i] = 14;
    }
    sort(poker1_temp,PUB_LEN);
    sort(poker2_temp,PUB_LEN);
    return compare_array(poker1_temp,poker2_temp,PUB_LEN);
}
int compare_poker_algo(int poker1[],int poker2[],int type){
    switch(type){
        case  POKER_TYPE_MAX_FLUSH_FLUSH :
           return 0;
        case  POKER_TYPE_FLUSH_STRAIGHT :
        case  POKER_TYPE_STRAIGHT :
           return compare_straight(poker1,poker2);
        case  POKER_TYPE_FOUR :
           return compare_four(poker1,poker2);
        case POKER_TYPE_FLUSH :
           return compare_flush(poker1,poker2);
        case  POKER_TYPE_THREE_KIND :
        case  POKER_TYPE_FULLHOUSE :
           return compare_three(poker1,poker2,type);
        case  POKER_TYPE_TWO_PAIR :
        case  POKER_TYPE_ONE_PAIR :
           return compare_two(poker1,poker2,type);
        case POKER_TYPE_A :
        case POKER_TYPE_SINGLE :
           return compare_one(poker1,poker2,type);
    }
    return -1;
}

