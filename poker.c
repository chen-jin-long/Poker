#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "poker.h"
#include "Utils.h"

int is_combine_straight(int pub[],int len1,int priv[],int len2,int *max_poker){
   //if(len2 < 2 || len1 <3) return -1;
   if(len2 < 2) return 0;
   int det = priv[1]-priv[0];
   int i = 0;
   if(det == 0)return 0;
   else if(det == 1){
      // 23 -> x ; a x c d ; so have 4 method
      int m1[3] = {priv[1]+1,priv[1]+2,priv[1]+3};
      int m2[3] = {priv[0]-1,priv[1]+1,priv[1]+2};
      int m3[3] = {priv[0]-2,priv[0]-1,priv[1]+1};
      int m4[4] = {priv[0]-3,priv[0]-2,priv[0]-1};
      int *p[4] = {m1,m2,m3,m4};
      int find_len = (int)sizeof(m1)/sizeof(int);
      for(i = 0;i<4;i++){
        if(1 == is_search(pub,len1,p[i],find_len)){
            //if(p[i][2] <= priv[1]) *max_poker = priv[1];
            // else *max_poker = p[i][2];
            *max_poker = get_max_poker(p[i],find_len,priv,len2);
            return 1;
        }
      }
  

   }else if(det == 2){
      // 3 5 --> 3 4 5 -->x ;  a  x  c  ;so have 3 method
     int mid[] = {priv[1] - 1};
     printf("det == 2,mid =%d",mid[0]);
     if(1 == is_search(pub,len1,mid,1)){
        int m1[2] = {priv[1]+1,priv[1]+2}; //x a c
        int m2[2] = {priv[0]-1,priv[1]+1}; // a x c
        int m3[2] = {priv[0]-2,priv[0]-1};
        int *p[] = {m1,m2,m3};
        int find_len = (int)sizeof(m1)/sizeof(int);
        for(i = 0;i<3;i++){
           if(1 == is_search(pub,len1,p[i],find_len)){
             // if(p[i][2] <= priv[1] ) *max_poker = priv[1];
             // else *max_poker = p[i][2];
             *max_poker = get_max_poker(p[i],find_len,priv,len2);
              return 1;
           }
        }
     }else{
        printf("not have mid =%d\n",mid[0]);
     }


   }else if(det == 3){
      // 3 6 --> 3 4 5 6 --> x a; a x ;so have 2 method
     int mid[] = {priv[0] +1,priv[0] +2};
     if(1 == is_search(pub,len1,mid,(int)sizeof(mid)/sizeof(int))){
          int m1[1] = {priv[1] + 1};
          int m2[1] = {priv[0] - 1};
          int *p[2] = {m1,m2};
          int find_len = (int)sizeof(m1)/sizeof(int);
          for(i = 0;i<2;i++){
              if(1 == is_search(pub,len1,p[i],find_len)){
                 *max_poker = get_max_poker(p[i],find_len,priv,len2);
                 return 1;
              }
          }
     }else{
       printf("not have mid...\n");
    }

   }else if(det == 4){
     int mid[] = {priv[0]+1,priv[0]+2,priv[0]+3};
     int find_len = (int)sizeof(mid)/sizeof(int);
     if(1 == is_search(pub,len1,mid,find_len)){
       *max_poker = get_max_poker(mid,find_len,priv,len2);
       return 1; 
     }
   }else if(det >= 9 && det <= 12 && priv[0] == 1){
    // 10 J Q K A ,NOTE: A is priv by player,
    // so there is 4 method 
    int mid_t[] = {0,0,0};
    int *mid = mid_t;
    printf("priv[1]= %d\n",priv[1]);
    if(priv[1] == 10){
       int  mid_t[] = {priv[1]+1,priv[1]+2,priv[1]+3};
       mid = mid_t;
    }else if(priv[1] ==11){
        int mid_t[] = {priv[1]-1,priv[1]+1,priv[1]+2};
        mid = mid_t;
    }else if(priv[1] == 12){
         printf("hahaha,12......\n");
        int mid_t[] = {priv[1]-2,priv[1]-1,priv[1]+1};
        mid = mid_t;
        printf("CGD mid[0] = %d\n",mid[0]);
    }else if(priv[1] == 13){
        int mid_t[] = {priv[1]-3,priv[1]-2,priv[1]-1};
        mid = mid_t;
    }else{
        printf("not can happed,error..");
    }

        printf("after CGD mid[0] = %d\n",mid[0]);
     if(1 == is_search(pub,len1,mid,(int)sizeof(mid)/sizeof(int))){
         *max_poker = 14;
         printf("hahaha,finded......\n");
         return 1;
     } 
   }
 return 0;
}

/*
int is_flush_game
return :
0: not flush
1: is flush ,not straight
2: is fush-starigh
*weight : used for compare straight value
*/
int is_flush_game(Person person,Game *game){

 int i = 0,count = 0,is_straight = 0,is_full_color = 0,max_straight = 0,type_result = 0;
 char color = person.priv[0].color;
 int priv[2] = {person.priv[0].value,person.priv[1].value};
 Poker pub[PUB_LEN];
 for(i = 0;i<PUB_LEN;i++)
    pub[i] = (*(game -> pub))[i];
/*
 for(i = 0;i<PRIV_LEN;i++)
    printf("person id = %d, priv[%d].value = %d,priv[%d].color = %c\n",person.id,i,person.priv[i].value,i,person.priv[i].color);

 for(i = 0;i<PUB_LEN;i++)
    printf("pub[%d],value = %d,color = %c\n",i,pub[i].value,pub[i].color);
*/
// if(person.priv[1].color - person.priv[0].color != 0)return 0;
 int *values = (int *)malloc(sizeof(int)*PUB_LEN);
 memset(values,0,sizeof(int)*PUB_LEN);
 if(person.priv[1].color - person.priv[0].color == 0){
    for(i = 0,count = 0;i < PUB_LEN;i++){
       if(pub[i].color == person.priv[0].color){
          values[count] = pub[i].value; 
          count ++;
       }
    }
    if(count < 3)is_full_color = 0;
    else is_full_color = 1; 
  //  free(values);
   // values = NULL;
    //return 0;
 }
// 判断是否是同花顺子
 if(is_full_color == 1){
    is_straight = is_combine_straight(values,count,priv,PRIV_LEN,&max_straight);
    if(is_straight == 1){
      // 是顺子 且是 同花
      printf(" is color-straight \n");
      if(max_straight == 14){
       // Poker max_chance_one[5] =  {{10,color},{11,color},{12,color},{13,color},{1,color}};
        // person.best_chance = &max_chance_one;
        for(i = 0;i<PUB_LEN;i++){
           (*person.best_chance + i)->value = 10+i;
           (*person.best_chance +i)->color = color;
         }
      }else{
        // Poker max_chance_two[5] = {{max_straight-4,color},{max_straight-3,color},{max_straight-2,color},{max_straight -1,color},{max_straight,color}};
        //person.best_chance = &max_chance_two;
        //(*person.best_chance + 4)->value = max_straight;
        for(i = 0;i<PUB_LEN;i++){
           (*person.best_chance + i)->value = max_straight-4+i;
           (*person.best_chance +i)->color = color;
        }
        //printf("max_chance_two = %d\n",max_chance_two[4]);
      }
      type_result = 3;
 }else{
   //不是顺子 但是 同花
   printf("is none-straight, just color ,count = %d\n",count);
   //Poker max_chance_three[5]={ {person.priv[0].value,color},{person.priv[1].value,color},{values[count-3],color},{values[count-2],color},{values[count-1],color}};
   //person.best_chance = &max_chance_three;
     for(i = 0;i<PUB_LEN;i++){
        if(i<2){
          (*person.best_chance + i)->value = priv[i];
        }else{
          (*person.best_chance + i)->value = values[count +i-5];
        }
        (*person.best_chance +i)->color = color;
     }
   type_result = 2;
  }
}else{
  is_straight = 0;
  is_straight = is_combine_straight(pub,PUB_LEN,priv,PRIV_LEN,&max_straight);
  if(is_straight){
    for(i = 0;i<PUB_LEN;i++){
        (*person.best_chance + i)->value = max_straight-4+i;
    }
   type_result = 1;
  }else{
   type_result = 0;
  }
}
 free(values);
 values = NULL;
 //return 1;
 return type_result;
}

int is_four_poker(Person person,Game *game){
   //如果私牌相同，在底牌中找两个相同的即可。
   //如果四牌不相同，在底牌中找三个相同的即可，存在两种情况
  int det = person.priv[1].value - person.priv[0].value;
  Poker pub[PUB_LEN];
  int i = 0,result = 0,max = 0;
  int pub_value[PUB_LEN];
  char pub_color[PUB_LEN];
  for(i = 0;i<PUB_LEN;i++){
     pub[i]= (*game->pub)[i];
     pub_value[i] = pub[i].value;
     pub_color[i] = pub[i].color;
  }
  int priv_value[PRIV_LEN] = {person.priv[0].value,person.priv[1].value};
  int priv_color[PRIV_LEN] = {person.priv[0].color,person.priv[1].color};  
  if(det == 0){
     int find[2] = {priv_value[0],priv_value[1]};     
     if(is_search(pub_value,PUB_LEN,find,2) == 1){
         printf("finded ,four poker\n");
         max = priv_value[0];
         result = 1;
     } 
  }else{
     int find_priv_one[3]= {priv_value[1],priv_value[1],priv_value[1]};
     int find_priv_zero[3]= {priv_value[0],priv_value[0],priv_value[0]};
     if(priv_value[0] == 1){
        for(i = 0;i<3;i++){
           find_priv_one[i] = priv_value[0];
           find_priv_zero[i]= priv_value[1];
        }
     }
     int temp_result = is_search(pub_value,PUB_LEN,find_priv_one,3);
     if(temp_result == 1){
         max = find_priv_one[0];
         printf("find,four poker from 3 pub,value = %d\n",max);
         result = 1;      
     }else{
        if(is_search(pub_value,PUB_LEN,find_priv_zero,3) == 1){
           result = 1; 
           max = find_priv_zero[1];
           printf("finded ,four poker from 3 pub,value = %d\n",max);
        }else{
          printf("sorry,we don't find for poker!\n");
        }
     }
  }
  if(result == 1){
      for(i = 0;i < 4;i++) (*person.best_chance + i)->value = max;
     if(det == 0){
        if(pub_value[0] != max & pub_value[0] == 1){
           (*person.best_chance +4)->value = 1;
        }else{
           for(i = PUB_LEN-1;i>=0;i--){
              if(pub_value[i] != max){
                 (*person.best_chance +4)->value = pub_value[i];
                 break;
              }
           }
        }
      }else{
          (*person.best_chance +4)->value = (max == priv_value[0]?priv_value[1]:priv_value[0]);
      }
   }
  return result;
}


int is_three_poker(Person person,Game *game){
  //先判断是不是 3+2,判断思想:将3+2看成两种情况，然后判断手上的两张牌处于哪种情况，这样就简化了，如果手上的两种牌相等会怎样，不等又会怎样
  //最后判断 3+1+1
  
  int det = person.priv[1].value - person.priv[0].value;
  int flag[PUB_LEN - 1];
  int i,k,max = 0,loc = 0,two_loc = 0,select;
  int temp_max = 0;
  int temp_finded = 0;
  int temp_count = 0;
  int constant_two ,single_one;
  for(i = 1;i<PUB_LEN;i++)
    flag[i-1]= (*game->pub)[i].value - (*game->pub)[i-1].value;
  int pub[PUB_LEN];
  for(i=0;i<PUB_LEN;i++)
     pub[i]= (*game->pub)[i].value; 
  int priv[PRIV_LEN]={person.priv[0].value,person.priv[1].value};
  if(det == 0){
    int find_one[1]= {person.priv[1].value};
    int three_value[2] = {0,0};//3poker:3pub,3poker:1pub+2priv
    // 3 = 3pub + 0priv
    //if(is_have_constant_three(flag,&loc) == 1){
    if(is_have_constant(flag,0,2,PUB_LEN-1,&loc)){
        three_value[0] = pub[loc];
        printf("have constant_three,3pub!!!!!!!!!!\n");
    }
    //3= 2priv + 1pub
    if(is_search_from_game(game,find_one,1) == 1){
        three_value[1] = person.priv[1].value;  
    }

    if(three_value[0] == 0 && three_value[1] == 0){
        printf("not find 3 poker!\n");
        return 0;
    }else{
        if((three_value[0] > three_value[1] && three_value[1] != 1 ) || three_value[0] == 1 ){
            for(i = 0;i < 3;i++){
              (*person.best_chance + i)->value =  three_value[0];
            }
            (*person.best_chance + 3)->value = priv[0];
            (*person.best_chance + 4)->value = priv[1];
        }else{
            printf("test...............\n");
            for(i = 0;i < 3;i++)
              (*person.best_chance + i)->value = three_value[1];
            /*
            for(i = PUB_LEN - 2;i > 0;i--){
                  if(flag[i] == 0){
                     temp_finded = 1;
                     max = pub[i];
                     break;
                  }
            }
            if(flag[0] == 0 && pub[0] == 1){
                max = 1;
            }
            */
            temp_finded = get_max_constant_num(pub,PUB_LEN,flag,PUB_LEN -1,&max);
            printf("max = %d\n",max);
            if(temp_finded == 1){  // 3+2
                  (*person.best_chance + 3)->value = max; 
                  (*person.best_chance + 4)->value = max; 
            }else{
                 int *out_poker = get_max_num_poker(pub,PUB_LEN,three_value[0],2);
                 (*person.best_chance + 3)->value = out_poker[0]; 
                 (*person.best_chance + 4)->value = out_poker[1]; 
                 free(out_poker);
                 printf("chenjinlong\n");
            }
      }
      return 1;
    }
    return 0;
  }else{
   // 3 in 3pub or2pub+ 1priv 
    //首先判断是否存在3pub
    // 思想：在pub的差数组中判断0的连续性个数即可，例如 0001 直接return 这是4 poker;0012,0100 存在两个连续的0，说明存在3pub;0102拿0与priv进行匹配  
   // 也就是说最多存在3个0，且不能是连续的3个0，
 
   int zero_num = cal_num_in_array(flag,PUB_LEN-1,0);
   if(zero_num == 3){
      for(i = 0;i<PUB_LEN-2;i++){
        // only:0010,0100
        if(flag[i] != 0){
           if(i == 2){
             constant_two = pub[i-1];
             single_one = pub[i+1];
           }else if(i == 1){
             constant_two = pub[i+1];
             single_one = pub[i-1];
           }else{
             printf("cant't happen,none-zero loc = %d\n",i);
           }
           break;
        }
      }
      printf("single_one = %d,constant_two = %d\n",single_one,constant_two);
      (*person.best_chance + 0)->value = constant_two;
      (*person.best_chance + 1)->value = constant_two;
      (*person.best_chance + 2)->value = constant_two;
      (*person.best_chance + 3)->value = priv[0];
      (*person.best_chance + 4)->value = priv[1];
      if(priv[0] == single_one || priv[1] == single_one){    // 3poker = 2pub+1priv
          if(single_one > constant_two || single_one == 1){  //for poker A 
            (*person.best_chance + 0)->value = single_one;
            (*person.best_chance + 1)->value = single_one;
            (*person.best_chance + 2)->value = single_one;
            if(priv[0] == single_one) (*person.best_chance + 3)->value = priv[1];
            else (*person.best_chance + 3)->value = priv[0];
            temp_max = 0;
            temp_max = get_bestchance_fourth_poker(pub,(*person.best_chance + 3)->value ,single_one);
            (*person.best_chance + 4)->value = pub[temp_max]; 
            /* ---------
            temp_max = 0;
            for(i = PUB_LEN-1;i>=0;i--){
               if(pub[i] == (*person.best_chance + 3)->value){
                  temp_max = pub[i];
                  break;
               }
            }
            if(temp_max != 0){
               (*person.best_chance + 4)->value = temp_max; 
            }else{
               for(i = PUB_LEN-1;i>=0;i--){
                  if(pub[i] != single_one){   // single_one in 3pub,so you just select not equals it
                     (*person.best_chance + 4)->value = pub[i]; 
                     break;
                  }
               }
            }
            ----------*/
          }
      }
      return 1; //have 3 poker;
   }else if(zero_num == 2 || zero_num == 1){
     int temp_loc2;
     if(is_have_constant(flag,0,2,PUB_LEN-2,&temp_loc2)){
            (*person.best_chance + 0)->value = pub[temp_loc2];
            (*person.best_chance + 1)->value = pub[temp_loc2];
            (*person.best_chance + 2)->value = pub[temp_loc2];
            (*person.best_chance + 3)->value = priv[0];
            (*person.best_chance + 4)->value = priv[1];
       return 1;
     }else{
         int three_result_temp = 0;
         for(i = PUB_LEN-2;i >= 0;i--){
           if(flag[i] == 0 && (pub[i]== priv[0] || pub[i] == priv[1])){
             three_result_temp = 1;
             (*person.best_chance + 0)->value = pub[i];
             (*person.best_chance + 1)->value = pub[i];
             (*person.best_chance + 2)->value = pub[i];
             if(priv[0] == pub[i]) (*person.best_chance + 3)->value = priv[1];
             else (*person.best_chance + 3)->value = priv[0];
             break;
           }
          }
          if(three_result_temp == 1){
            temp_max = 0;
            temp_max = get_bestchance_fourth_poker(pub,(*person.best_chance + 3)->value ,pub[i]);
            (*person.best_chance + 4)->value = pub[temp_max]; 
            //compare four and five value for poker A,
            if((*person.best_chance +3)->value > (*person.best_chance +4)->value){
                int temp_value = (*person.best_chance +3)->value;
                (*person.best_chance + 3)->value = (*person.best_chance +4)->value;
                (*person.best_chance + 4)->value = temp_value;
            }
            if(pub[0] == 1 && pub[0] != pub[i]){
              (*person.best_chance + 3)->value = 1;
            }
           return 1;
         }
     }
   } 
 } 

 return 0;
}

/*
return 1: only one two-poker
return 2: two two-poker

*/
int is_two_poker(Person person,Game *game){
  int i,count,max,zero_num;
  int flag[PUB_LEN - 1];
  int pub[PUB_LEN];
  int two_poker[2] = {0,0};
  int det = person.priv[1].value - person.priv[0].value;
  for(i = 1;i < PUB_LEN;i++)
     flag[i-1]= (*game->pub)[i].value - (*game->pub)[i-1].value;

  for(i = 0;i < PUB_LEN;i++)
     pub[i]= (*game->pub)[i].value; 

  int priv[PRIV_LEN]={person.priv[0].value,person.priv[1].value};
  zero_num = cal_num_in_array(flag,PUB_LEN-1,0);
  if(det == 0){
      two_poker[0] = priv[0];
      /*
      if(flag[0] == 0 && pub[0] == 1) two_poker[1] = 1;
      else{
       for(i = PUB_LEN-2,count = 0;i >= 0;i--){
         if(flag[i] == 0){
             two_poker[1] = pub[i];
             break;
         }
       }
      }
      */
      get_max_constant_num(pub,PUB_LEN,flag,PUB_LEN -1,&two_poker[1]);
      (*person.best_chance + 0 )->value = two_poker[0];
      (*person.best_chance + 1 )->value = two_poker[0];
      if(two_poker[1] == 0){
          if(pub[0] == 1){
             (*person.best_chance + 2 )->value = 1;
             for(i=PUB_LEN-1,count = 0;i>=0;i--){
               (*person.best_chance +3 +count)->value = pub[i];
               count ++;
               if(count == 2 )break;
             }
          }else{
            for(i=PUB_LEN-1,count = 0;i>=0;i--){
               (*person.best_chance +2+count)->value = pub[i];
               count ++;
               if(count == 3 )break;
             }
           }
         return 1;
      }else{
          (*person.best_chance + 2 )->value = two_poker[1];
          (*person.best_chance + 3 )->value = two_poker[1];
          if(two_poker[0] != 1 && two_poker[1] != 1 && pub[0] == 1)
             (*person.best_chance + 4 )->value = 1;
          else{
             for(i = PUB_LEN-1,count = 0;i >= 0;i--){
               if(pub[i] != two_poker[0] && pub[i] != two_poker[1]){
                  (*person.best_chance + 4)->value = pub[i];
                  break;
               }
          }
       }
        return 2;
      }
      return 1;
  }else{
     int chance[3] = {0,0,0};// aa,bb,cc
     /*
     for(i = 0;i< PUB_LEN-1;i++){
        if(priv[0] == pub[i]) chance[0] = pub[i];
        else if(priv[1] == pub[i]) chance[1] = pub[i];
     }
     for(i = PUB_LEN-2;i>=0;i--){
        if(flag[i] == 0){
           chance[2] = pub[i];
           break;
        }
     }
     if(flag[0] == 0 && pub[0] == 1)chance[2] = 1;
     */
     get_max_constant_num(pub,PUB_LEN,flag,PUB_LEN -1,&chance[2]);
     printf("chance is %d,%d,%d\n",chance[0],chance[1],chance[2]);
     zero_num =  cal_num_in_array(chance,3,0);
     printf("zero_num = %d\n",zero_num);
     if(zero_num == 0){ 
       if(chance[2] == 1){
          (*person.best_chance + 0 )->value = chance[2];
          (*person.best_chance + 1 )->value = chance[2];
          (*person.best_chance + 2 )->value = chance[1];
          (*person.best_chance + 3 )->value = chance[1];
          (*person.best_chance + 4 )->value = chance[0];

       }else{
           if(chance[0] == 1 && chance[1] != 0){
                 if(chance[2] > chance[1]){
                    (*person.best_chance + 0 )->value = chance[0];
                    (*person.best_chance + 1 )->value = chance[0];
                    (*person.best_chance + 2 )->value = chance[2];
                    (*person.best_chance + 3 )->value = chance[2];
                    (*person.best_chance + 4 )->value = chance[1];
                 }else{
                    (*person.best_chance + 0 )->value = chance[0];
                    (*person.best_chance + 1 )->value = chance[0];
                    (*person.best_chance + 2 )->value = chance[1];
                    (*person.best_chance + 3 )->value = chance[1];
                    (*person.best_chance + 4 )->value = chance[2];
                 }

           }else if(chance[0] != 1 && chance[1] == 1){
                if(chance[2] > chance[0]){
                    (*person.best_chance + 0 )->value = chance[1];
                    (*person.best_chance + 1 )->value = chance[1];
                    (*person.best_chance + 2 )->value = chance[2];
                    (*person.best_chance + 3 )->value = chance[2];
                    (*person.best_chance + 4 )->value = chance[0];
                }else{
                    (*person.best_chance + 0 )->value = chance[1];
                    (*person.best_chance + 1 )->value = chance[1];
                    (*person.best_chance + 2 )->value = chance[0];
                    (*person.best_chance + 3 )->value = chance[0];
                    (*person.best_chance + 4 )->value = chance[2];
                }

           }else{ //none poker 1
             if(chance[2] < chance[0]){
               (*person.best_chance + 0 )->value = chance[0];
               (*person.best_chance + 1 )->value = chance[0];
               (*person.best_chance + 2 )->value = chance[1];
               (*person.best_chance + 3 )->value = chance[1];
               (*person.best_chance + 4 )->value = chance[2];
             }else if(chance[2] > chance[1]){
               (*person.best_chance + 0 )->value = chance[2];
               (*person.best_chance + 1 )->value = chance[2];
               (*person.best_chance + 2 )->value = chance[1];
               (*person.best_chance + 3 )->value = chance[1];
               (*person.best_chance + 4 )->value = chance[0];
             }else if(chance[2] > chance[0] && chance[2] < chance[1]){
               (*person.best_chance + 0 )->value = chance[1];
               (*person.best_chance + 1 )->value = chance[1];
               (*person.best_chance + 2 )->value = chance[2];
               (*person.best_chance + 3 )->value = chance[2];
               (*person.best_chance + 4 )->value = chance[0];
             }else{
                printf("in_two_poker error!!!\n");
             }
          }
       }
       return 2;
     }else if(zero_num == 1){
       for(i = 0,count = 0;i<3;i++){
         if(chance[i] != 0){
           (*person.best_chance + count )->value = chance[i];
           (*person.best_chance + count + 1 )->value = chance[i];
           count += 2;
         }else{
           (*person.best_chance + 4 )->value = chance[i];
         }
       }
       return 2;
     }else if(zero_num == 2){
       if(chance[2] != 0){
          (*person.best_chance + 0 )->value = chance[2];
          (*person.best_chance + 1 )->value = chance[2];
          (*person.best_chance + 2 )->value = priv[1];
          (*person.best_chance + 3 )->value = priv[0];
          if(chance[2] != 1 && pub[0] == 1)
              (*person.best_chance + 4 )->value = 1;
          else{
             for(i=PUB_LEN-1;i>=0;i--){
                if(pub[i] != chance[2]){
                  (*person.best_chance + 4 )->value = pub[i];
                  break;
                }
              }
           }
       }else{
          if(chance[0] != 0){
            (*person.best_chance + 0 )->value = chance[0];
            (*person.best_chance + 1 )->value = chance[0];
            (*person.best_chance + 2 )->value = priv[1];
            if(chance[0] != 1 && pub[0] == 1){
              (*person.best_chance + 3 )->value = 1;
              for(i=PUB_LEN-1,count = 0;i>=0;i--){
                if(pub[i] != chance[0]){
                  (*person.best_chance + 4+count )->value = pub[i];
                  count ++;
                  if(count == 1)break;
                }
              }
            }else{
              for(i=PUB_LEN-1,count = 0;i>=0;i--){
                if(pub[i] != chance[0]){
                  (*person.best_chance + 3+count )->value = pub[i];
                  count ++;
                  if(count == 2)break;
                }
              }
           }

          }else if(chance[1] != 0){
            (*person.best_chance + 0 )->value = chance[1];
            (*person.best_chance + 1 )->value = chance[1];
            (*person.best_chance + 2 )->value = priv[0];
            if(chance[1] != 1 && pub[0] == 1){
              (*person.best_chance + 3 )->value = 1;
              for(i=PUB_LEN-1,count = 0;i>=0;i--){
                if(pub[i] != chance[0]){
                  (*person.best_chance + 4+count )->value = pub[i];
                  count ++;
                  if(count == 1)break;
                }
              }
            }else{
              for(i=PUB_LEN-1,count = 0;i>=0;i--){
                if(pub[i] != chance[1]){
                  (*person.best_chance +3 + count )->value = pub[i];
                  count ++;
                  if(count == 2)break;
                }
              }
           }
          } 
       } 
       return 1;

     }else if(zero_num == 3){
        (*person.best_chance + 0)->value = priv[0];
        (*person.best_chance + 1)->value = priv[1];
        if(pub[0] == 1){
          (*person.best_chance + 2)->value = 1;
          for(i=PUB_LEN-1,count = 0;i>=0;i--){
             (*person.best_chance +3+count)->value = pub[i];
             count ++;
             if(count == 2 )break;
          }
        }else{
          for(i=PUB_LEN-1,count = 0;i>=0;i--){
             (*person.best_chance +2+count)->value = pub[i];
             count ++;
             if(count == 3 )break;
          }
        }
       return 0;
     }

  }
  return 0;
}

#if 0
void main(){
//test_is_search();
//test_is_search();
test_is_combine_search();
}
#endif
