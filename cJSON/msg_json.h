#ifndef __POKER_MSG_JSON__
#define __POKER_MSG_JSON__

typedef struct {
    int clientSN;
    unsigned int serialCode;
    int msgTotalNum;
}Poker_Msg_Header;

typedef struct {
    char * msgType;
    char * msgValue;
}Poker_Msg_Body;

typedef struct msg_json
{
    int timestamp;
    char endTag[4];
}Poker_Msg_Tail;

typedef struct  {
    Poker_Msg_Header *msgHeader;
    Poker_Msg_Body *msgBody;
    Poker_Msg_Tail *msgTail;
}Poker_Msg_Module;

typedef struct {
    char magic[9];
    int len; //16进制
    char *info;
}Poker_Msg_STU;

typedef struct
{
  int connId;
  //INFO info;
  Poker_Msg_Module *module;
}QueueMsg;

#define POKER_MSG_MAGIC "a1b2c3d4"
#define POKER_MSG_MAGIC_LEN 8

#define POKER_MSG_END_TAG "end"
#define POKER_MSG_END_TAG_LEN 3

#define POKER_MSG_HEADER_CLIENT_SN    "clientSN:"
#define POKER_MSG_HEADER_SERIAL_CODE  "serialCode:"
#define POKER_MSG_HEADER_TOTAL_NUM    "msgTotalNum:"
#define POKER_MSG_BODY_TAG            "body:"
#define POKER_MSG_BODY_TYPE           "type:"
#define POKER_MSG_BODY_VALUE          "value:"
#define POKER_MSG_TAIL_TIMESTAMP      "timeStamp:"
#define POKER_MSG_TAIL_END_TAG        "endTag:"

#define MAX_POKER_MSG_NUM 2


//由于我们使用了snprintf(buf, len, "%s%04x%s",所以不能直接用sizeof
//#define HEADER_POKER_MSG_LEN (sizeof(Poker_Msg_STU))
#define HEADER_POKER_MSG_LEN (POKER_MSG_MAGIC_LEN + sizeof(int))

char * build_poker_msg(int clientSN, const char *action, const char *info);
int parseMsgHeader(char *buf, int *len);
Poker_Msg_Module * parseJsonMsg(char *json_info, int json_len);
void free_poker_msg_module(Poker_Msg_Module *module);

#endif