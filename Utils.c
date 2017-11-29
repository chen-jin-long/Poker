#include <stdio.h>
#include "poker.h"

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
    int i = 0,flag = 0,j=0,is_A = 0;
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
        is_A = i;
        find[i] = 1;// to sovle A ; 
     }
      while(j < len1){
         if(base[j] == find[i] && f[j] == 0){
              flag = 1;
              f[j] = 1;
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
    if(is_A != 0){
       find[is_A] = 14;
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
               return best_loc;
           }
     }
 
     for(i = PUB_LEN-1;i>=0;i--){
         if(pub[i] != diff){   // single_one in 3pub,so you just select not equals it
            best_loc = i;
            break;
         }
      }
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
  if(pub_len < flag_len)return max;
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



