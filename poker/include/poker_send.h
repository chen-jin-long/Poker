#ifndef _POKER_SEND_
#define _POKER_SEND_

#ifdef __cplusplus
extern "C" {
#endif

Poker * wash_poker();
Poker *get_fixed_poker(const char *buffer);
Poker * get_fixed_poker_from_file(const char * name);


#ifdef __cplusplus
}
#endif

#endif
