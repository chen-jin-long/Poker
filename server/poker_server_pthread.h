#ifndef __POKER_THREAD_SERVER__
#define __POKER_THREAD_SERVER__

#include <stdio.h>
#include "common.h"

typedef struct {
    int clientSN;
    int gameType;
    int roomId;
    int deskId;
} GamePlayerDataBase;


#endif