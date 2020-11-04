#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "poker.h"
void print_array(int poker[],int len);
void print_Array(const char * name,int poker[],int len);

int is_find(int base[],int len,int value){
    int i=0;
    for(i=0;i<len;i++){
       if(base[i] == value)return 1;
    }
    return 0;
}

int is_search(int base[],int len1,int find[],int len2){
    //printf("====len1 = %d========\n",len1);
    //int i ,flag ,j,is_A;
    print_Array("base",base,len1);
    print_Array("find",find,len2);
    printf("len2 = %d\n",len2);
    int i = 0,flag = 0,j=0,is_A = 0,loc_A = 0;
    //for(i = 0;i<len2;i++)printf("find[%d] = %d\n",i,find[i]);
    int * f =NULL;
    f =(int *) malloc(sizeof(int)*len1);
    if(f != NULL) memset(f,0,sizeof(int)*len1);
    else return -1;
 
    i = 0;
    while(i < len2){
      flag = 0;
      if(find[i] < 1 || find[i] >14)return -2;
      else if(find[i] == 14){
        is_A = 1;
        loc_A = i;
        find[i] = 1;// to sovle A ; 
      }
      while(j < len1){
         if(base[j] == find[i] && f[j] == 0){
              flag = 1;
              f[j] = 1; //处理查找两个相同的值的情况，必须将查找到的做个标记
     //         printf("finded ;find[%d] = %d,\n",i,find[i]);
              break;
         }else{
 
       //       printf("base= %d,find = %d,f = %d,i = %d,j = %d\n",base[j],find[i],f[j],i,j);
         }
         j++;
      }
      if(flag  == 0) break;
      i++;
      j = 0;
    }
    free(f);
    f = NULL;
    if(is_A == 1){
       find[loc_A] = 14;
       is_A = 0;
    }
    
    if(flag == 1)return 1;
    else return 0;
    
    }

    int is_search_from_game(Game *game,int find[],int len){
       int pub[PUB_LEN];
       int i;
       for(i = 0;i<PUB_LEN;i++){
            pub[i] = (*game->pub)[i].value;
       }
      return is_search(pub,PUB_LEN,find,len); 
    }
    
    /*
     this error, please not use this function
    */
    int is_have_constant_three(int flag[],int *loc){
      int i,count;
      for(i = 0,count = 0;i<PUB_LEN-1;i++){
         if(flag[i] == 0){
           count ++;
         }else{
           //在flag中有两个连续的零就说明公共牌中含有3张一样的poker
            if(count == 2){
          *loc =  i-2;
              printf("is_have_constant_tree:true,loc = %d\n",i-2);
              return 1;
            }else count = 0;
         }
      }
    
    }
    

int  get_max_poker(int pub[],int len1,int priv[],int len2){
    int max_poker = 0;
    if(pub[len1 - 1] <= priv[1]) max_poker = priv[len2 - 1];
    else max_poker = pub[len1 - 1];
    return max_poker;
}
/*

is the same color

*/
int is_flush(int pub[],int len1,int priv[],int len2,int * max_poker){
    if(priv[1]- priv[0] != 0 )return 0;
    int i,count;
   // int * find= (int *)malloc(sizeof(int)*len1);
    for(i = 0,count = 0;i<len1;i++){
       if(pub[i] == priv[0]){
          //find[count] = pub
          count++;
       }
    }   
    if(count<3)return 0;
 
}
 int cal_num_in_array(int flag[],int len,int value){
   int i = 0,count = 0;
   for(i=0;i<len;i++){
     if(flag[i] == value) count++;
   }
  return count;
 }
 
 int get_bestchance_fourth_poker(int pub[],int same ,int diff){
     int best_loc = 0,i;
     for(i = PUB_LEN-1; i >= 0;i--){
          if(pub[i] == same){
               best_loc = i;
               LOG_DEBUG("have same values,same = %d,loc= %d",same,i);
               return best_loc;
           }
     }
 
     for(i = PUB_LEN-1;i>=0;i--){
         if(pub[i] != diff){   // single_one in 3pub,so you just select not equals it
            best_loc = i;
            break;
         }
      }
     LOG_DEBUG("best_loc = %d",best_loc);
     return best_loc;
 }
 int * get_max_num_poker(int pub[],int len,int diff,int num){
    int i,count;
    int *out = (int *)malloc(sizeof(int)*num);
    for(i = len-1,count = 0;i >= 0;i--){
        if(pub[i] != diff){
          out[count] = pub[i];
          count++;
          if(count == num)
            break;
        }
    }
    if(pub[0] == 1){
      out[num-1] = 1;
    }
    return out;
 } 

int get_max_constant_num(int pub[],int pub_len,int flag[],int flag_len,int *max){
  int find = 0,i;
  if(pub_len < flag_len)return -1;
  print_array(flag,flag_len);
  for(i = flag_len-1;i >= 0;i--){
    if(flag[i] == 0){
       *max = pub[i];
       find = 1;
       break;
    }
 }
 if(flag[0] == 0 && pub[0] == 1)*max = 1;
 return find;
}


/*
 Note: end reprent num count ;if you begin from 0,please +1
*/
int is_have_constant(int flag[],int value,int nums,int end,int *loc){
  int i,count;
  if(end > PUB_LEN) return -1;
//  for(i = 0;i<PUB_LEN-1;i++)
  //   printf("flag[%d] = %d\n",i,flag[i]);
  for(i = end-1,count = 0;i >= 0;i--){
     if(flag[i] == value){
        count ++;
        //printf("=== i = %d ====\n",i);
        if(count == nums){
          //*loc = i;
          *loc = i;
          printf("[count:%d,nums:%d]you have find: %d,nums = %d,loc = %d, end = %d\n",count,nums,value,nums,*loc,end);
          return 1;
        }
     }else{
          count = 0;
     }
   }
return 0;
}
void sort(int poker[],int len){
  int i,k,temp;
  for(i = len-1;i > 0;i--){
    for(k = i-1;k >=0;k--){
       if(poker[i] < poker[k]){
           temp = poker[k];
           poker[k] = poker[i];
           poker[i] = temp;
       }
    }
  }

}

void sort_and_color(int poker[],char color[],int len){
  int i,k,temp;
  char temp_color;
  for(i = len-1;i > 0;i--){
    for(k = i-1;k >=0;k--){
       if(poker[i] < poker[k]){
           temp = poker[k];
           poker[k] = poker[i];
           poker[i] = temp;
           temp_color = color[k];
           color[k] = color[i];
           color[i] = temp_color;
       }
    }
  }
}

void sort_poker_game(Person *person,Game *game){
  int pub[PUB_LEN]={0};
  char pub_color[PUB_LEN] = {'s'};
  int priv[PRIV_LEN] = {0};
  char priv_color[PRIV_LEN] = {'s'};
  int i = 0;
  for(i = 0;i < PRIV_LEN;i++){
    priv[i] = (person->priv)[i].value;
    priv_color[i] = (person->priv)[i].color;
  }
  sort_and_color(priv,priv_color,PRIV_LEN); 
  for(i = 0;i < PRIV_LEN;i++){
    (person->priv)[i].value = priv[i];
    (person->priv)[i].color = priv_color[i];
  }

  for(i = 0;i< PUB_LEN;i++){
   pub[i] = (*(game->pub))[i].value;
   pub_color[i] = (*(game->pub))[i].color;
  }
  sort_and_color(pub,pub_color,PUB_LEN);
  
  for(i = 0;i< PUB_LEN;i++){
   (*(game->pub))[i].value = pub[i];
   (*(game->pub))[i].color = pub_color[i];
  }
}

void sort_poker_bestChance(Poker (*best_chance)[PUB_LEN])
{
    int p_value[PUB_LEN] = {0};
    char p_color[PUB_LEN] = {'s'};
    int i = 0;
    for(i = 0; i < PUB_LEN; i++){
        p_value[i] = (*best_chance)[i].value;
        p_color[i] = (*best_chance)[i].color;
    }
    sort_and_color(p_value,p_color,PUB_LEN);
    for(i = 0; i < PUB_LEN; i++) {
        //(*best_chance[i]).value = p_value[i];
        //(*best_chance[i]).color = p_color[i];
        //best_chance[i]->value = p_value[i];
        //best_chance[i]->color = p_color[i];

        (*best_chance)[i].value = p_value[i];
        (*best_chance)[i].color = p_color[i];
        //(*best_chance + i)->value = p_value[i];
        //(*best_chance + i)->color = p_color[i];
    }
}

void print_array(int poker[],int len){
  int i=0;
  for(i = 0;i<len;i++){
    printf("poker[%d] = %d ,",i,poker[i]);
  }
  printf("\n");
}
void print_color(char color[],int len){
  int i=0;
  for(i = 0;i<len;i++){
    printf("color[%d] = %c ,",i,color[i]);
  }
  printf("\n");
}

void print_Array(const char * name,int poker[],int len){
  int i=0;
  for(i = 0;i<len;i++){
    printf("%s[%d] = %d ,",name,i,poker[i]);
  }
  printf("\n");
}

void print_Poker(Poker *poker) {
    if (poker) {
        printf("[%d:%c]\n", poker->value, poker->color);
    }
}

void print_BestChance(Poker *poker, int len) {
    int i = 0;
    printf("=====Best Choice====\n");
    for (i = 0; i < len; i++) {
       print_Poker(poker+i);
    }
    printf("=====Best Choice====\n");
}
void get_three_poker_val(int poker[],int *value){
        int i = 0,count = 0;
        int flag[PUB_LEN -1];
        for(i = 0;i < PUB_LEN-1;i++){
           flag[i] = poker[i+1] -poker[i];
        }
        for(i = 0,count = 0;i< PUB_LEN - 2;i++){
            if(flag[i] == flag[i+1] && flag[i] == 0){
               value[0] = poker[i];
               break;
            }
        }
        //printf("three_value:%d\n",value[0]);
        for(i = 0,count = 1;i< PUB_LEN;i++){
            if(poker[i] != value[0]){
                   value[count]  = poker[i];
                   if(count == 2)break;
                   count++;
            }
        }
}

int find_poker(Poker *base,Poker *find)
{
  int i = 0;
  if(base == NULL || find == NULL)
  {
     return 0;
  }

  for(i = 0;i<ONE_UNIT_POKER;i++)
  {
    if((find->value == (base+i)->value) && (find->color == (base+i)->color))
    {
       return 1;
    }
  }
  return 0;
}

int match_pub_poker_color(Poker *poker, Game *game, int *flag)
{
    int i = 0;
    for (i = 0; i < PUB_LEN; i++) {
        if ((poker->value == (*game->pub + i)->value) && flag[i] == 0) {
            poker->color = (*game->pub + i)->color;
            flag[i] = 1;
            return 0;
        }
    }
    return -1;
}

int set_bestChance_color(Person *person, Game *game, int result)
{
    int matchFlag[PUB_LEN+PRIV_LEN] = {0};
    int i = 0;
    if (person == NULL || game == NULL) {
        return -1;
    }
    if (result >= POKER_TYPE_FLUSH_STRAIGHT) {
        printf("[%s] result =%d, not set bestchance color.\n", __FUNCTION__, result);
        return 0;
    }
    for (i = 0; i < PUB_LEN; i++) {
        if ((*person->best_chance + i)->value == person->priv[0].value && matchFlag[5] == 0) {
            (*person->best_chance + i)->color = person->priv[0].color;
            matchFlag[5] = 1;
        } else if ((*person->best_chance + i)->value == person->priv[1].value && matchFlag[6] == 0) {
            (*person->best_chance + i)->color = person->priv[1].color;
            matchFlag[6] = 1;
        } else {
            if (match_pub_poker_color(*person->best_chance + i,  game, matchFlag)) {
                printf("[%s] error..\n", __FUNCTION__);
                return -1;
            }
        }

    }
    return 0;
}

int getClientId(const char *data)
{

}

