#include<string.h>
#include<stdlib.h>
#include "common.h"
#include "msg_json.h"

void dumpPrivMsg(const char *msg)
{
  Req_Poker *reqPoker = NULL;
  Req_Poker_Header head;
  reqPoker->head = &head;
  int index = 0;
  int len = strlen(msg);
  char temp[256] = {0};
  strncpy(temp,msg,sizeof(temp)-1);
  int i = 0;
  for(i = 0;i < len;i++)
  {
    if(temp[i] == '0')
    {
      temp[i] = 0;
    }
  }
  memcpy(&reqPoker->head->command,temp,sizeof(reqPoker->head->command));
  index += sizeof(reqPoker->head->command);
  memcpy(&reqPoker->head->user_id,temp+index,sizeof(reqPoker->head->user_id));
  index += sizeof(reqPoker->head->user_id);
  memcpy(&reqPoker->head->len,temp+index,sizeof(reqPoker->head->len));
  index += sizeof(reqPoker->head->len);
  printf("command : %d ",reqPoker->head->command);
  printf("user_id: %d ",reqPoker->head->user_id);
  printf("len: %d \n",reqPoker->head->len); 
  Poker *p = (Poker *)malloc(reqPoker->head->len);
  if(p == NULL) return;
  memcpy(p,temp+index,sizeof(Poker)*reqPoker->head->len); 
  for(i = 0; i < reqPoker->head->len;i++)
  {
    printf("poker: value: %d ,color = %c\n",(p+i)->value,(p+i)->color); 
  }
  free(p);

}
POKER_RETURE_PARAM get_poker(Poker *poker,char *out)
{
   if(poker == NULL || out == NULL)
   {
       return POKER_PARAM_NULL_ERROR;
   }
   snprintf(out,4,"%02d%c",poker->value,poker->color);
   return POKER_OK;

}

POKER_RETURE_PARAM get_poker_msg(Poker *poker, int num, char *out)
{
   if(poker == NULL || out == NULL)
   {
       return POKER_PARAM_NULL_ERROR;
   }
   int i = 0;
   for (i = 0; i < num; i++) {
        snprintf(out+(i*3),4,"%02d%c",(poker+i)->value,(poker+i)->color);
   }

   return POKER_OK;

}

void parseRecvMsg(PokerMsgBuf *msgBuf, char *recv_buf, int recv_len, HandlerJsonMsgCb handler)
{
  // 暂存临时缓存
  char tmp_buf[MAX_RECV_LEN] = {0};
  // 消息指针
  char *dataMsg = NULL;
  int msg_len = 0;
  if (msgBuf == NULL || recv_buf == NULL) {
    return;
  }
  printf("[%s]recv_len = %d\n", __FUNCTION__, recv_len);
  // 消息指针指向待处理数据
  dataMsg = msgBuf->remain_buf;
  // 防止溢出，但是不会溢出，因为recv的最大长度是TMP_BUF_LEN, 远远小于MAX_RECV_LEN
  if (recv_len >= ((int)sizeof(msgBuf->remain_buf) - msgBuf->remain_len)) {
    recv_len = (int)sizeof(msgBuf->remain_buf) - msgBuf->remain_len - 1;
    printf("[%s] override recv buf.\n", __FUNCTION__);
  }
  if (msgBuf->remain_len < (int)sizeof(msgBuf->remain_buf)) {
    strncpy(msgBuf->remain_buf + msgBuf->remain_len, recv_buf, recv_len);
    // 判断读到的消息或者剩余的是否达到一个消息头的长度
    msgBuf->remain_len += recv_len;
  } else {
    printf("[%s] override remain buf.\n", __FUNCTION__);
  }
  // 大于就行，不用等于，我们的目标是处理完整消息。
  while (msgBuf->remain_len >  HEADER_POKER_MSG_LEN) {
    if (dataMsg && parseMsgHeader(dataMsg, &msg_len) == 0) {
          //判断消息体长度是否大于剩余长度
      printf("[%s]msg_len = %d, remain_len = %d\n", __FUNCTION__, msg_len, msgBuf->remain_len);
      if (msg_len <= (msgBuf->remain_len - HEADER_POKER_MSG_LEN)) {
              // 减少缓存使用，直接将一个完整消息丢入即可，但是不能影响其他消息队列的处理，需要注意！！！
              char *msgBody = dataMsg + HEADER_POKER_MSG_LEN;
              Poker_Msg_Module *module = parseJsonMsg(msgBody, msg_len);
              if (module) {
                QueueMsg * queue_msg = (QueueMsg *)malloc(sizeof(QueueMsg));
                if (queue_msg) {
                  queue_msg->connId = msgBuf->connId;
                  queue_msg->module = module;
                  handler(queue_msg);
                }
              } else {
                printf("[%s] module is NULL.\n", __FUNCTION__);
              }
              msgBuf->remain_len -= (HEADER_POKER_MSG_LEN + msg_len);
              dataMsg += (HEADER_POKER_MSG_LEN + msg_len);
              msg_len = 0;
              continue;
      } else {
        // 剩余字符不够一个完整消息，会影响到while的判断，不能单纯的break
        // 怎么办？ 简单处理，不够一个完整消息，就不处理，直到形成一个完整的消息
        printf("[%s] error to break.\n", __FUNCTION__);
        break;
      }
    } else {
        // 不应该运行到这，按照设计原则： 必须连续处理一个完整包，否则就等结尾补上来， magic不可能判断错误，运行到这就完蛋了！！！
          memset(msgBuf->remain_buf, 0, sizeof(msgBuf->remain_buf));
          msgBuf->remain_len = 0;
          printf("[%s] error to run here.\n", __FUNCTION__);
          return;
    }
  }
  // 消息追加,将未处理的消息通过recv_buf临时转存。
  if (dataMsg) {
    strncpy(tmp_buf, dataMsg, sizeof(tmp_buf)-1);
    memset(msgBuf->remain_buf, 0, sizeof(msgBuf->remain_buf));
    strncpy(msgBuf->remain_buf, tmp_buf, sizeof(msgBuf->remain_buf)-1);
  }
}
