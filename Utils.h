#ifndef _POKER_UTILS_
#define _POKER_UTILS_
#include "poker.h"
int is_find(int base[],int len,int value);
int is_search(int base[],int len1,int find[],int len2);
int  get_max_poker(int pub[],int len1,int priv[],int len2);
int is_flush(int pub[],int len1,int priv[],int len2,int * max_poker);
int cal_num_in_array(int flag[],int len,int value);
int get_bestchance_fourth_poker(int pub[],int same ,int diff);
int * get_max_num_poker(int pub[],int len,int diff,int num);
int get_max_constant_num(int pub[],int pub_len,int flag[],int flag_len,int *max);
int is_have_constant(int flag[],int value,int nums,int end,int *loc);
void sort(int poker[],int len);
void print_array(int poker[],int len);
void print_Array(const char * name,int poker[],int len);
void print_color(char color[],int len);
void get_three_poker_val(int poker[],int *value);
int sort_poker_game(Person *person,Game *game);
#endif
