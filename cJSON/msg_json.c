#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include <fcntl.h>
#include <unistd.h>

#include "cJSON.h"
#include "msg_json.h"

int getRandIntByDev(unsigned int *data);

cJSON *build_poker_json_msg(Poker_Msg_Module *module)
{
    int i = 0;
    cJSON *json_msg = cJSON_CreateObject();
    if (json_msg == NULL) {
        return NULL;
    }

    Poker_Msg_Header *header = module->msgHeader;
    cJSON *clientSN = cJSON_CreateNumber(header->clientSN);
    cJSON_AddItemToObject(json_msg, POKER_MSG_HEADER_CLIENT_SN, clientSN);
    cJSON *serialCode = cJSON_CreateNumber(header->serialCode);
    cJSON_AddItemToObject(json_msg, POKER_MSG_HEADER_SERIAL_CODE, serialCode);
    cJSON *msgTotalNum = cJSON_CreateNumber(header->msgTotalNum);
    cJSON_AddItemToObject(json_msg, POKER_MSG_HEADER_TOTAL_NUM, msgTotalNum);
    cJSON *body = cJSON_CreateArray();
    cJSON_AddItemToObject(json_msg, POKER_MSG_BODY_TAG, body);


    //for (i = 0; i < header->msgTotalNum; i++) {
        Poker_Msg_Body *msgBody = module->msgBody;
        cJSON *msgInfo = cJSON_CreateObject();
        cJSON_AddItemToArray(body, msgInfo);
        cJSON * msgType = cJSON_CreateString(msgBody->msgType);
        cJSON_AddItemToObject(msgInfo, POKER_MSG_BODY_TYPE, msgType);
        //cJSON *msgLen = cJSON_CreateNumber(msgBody->msgLen);
        //cJSON_AddItemToObject(msgInfo, "len:", msgLen);
        cJSON *msgValue = cJSON_CreateString(msgBody->msgValue);
        cJSON_AddItemToObject(msgInfo, POKER_MSG_BODY_VALUE, msgValue);
    //}

    Poker_Msg_Tail *msgTail = module->msgTail;
    //cJSON *tail = cJSON_CreateObject();
    cJSON *stamp = cJSON_CreateNumber(msgTail->timestamp);
    cJSON_AddItemToObject(json_msg, POKER_MSG_TAIL_TIMESTAMP, stamp);
    cJSON *tag = cJSON_CreateString(msgTail->endTag);
    cJSON_AddItemToObject(json_msg, POKER_MSG_TAIL_END_TAG, tag);

    return json_msg;
}

Poker_Msg_Module * build_poker_msg_module(int clientSN, const char *action, const char *info)
{
    Poker_Msg_Header *header = NULL;
    Poker_Msg_Body *body = NULL;
    Poker_Msg_Tail *tail = NULL;
    Poker_Msg_Module *module = NULL;
    unsigned int seed = 0;
    int ret = 0;
    if (action == NULL || info == NULL) {
        return NULL;
    }

    ret = getRandIntByDev(&seed);
    if (ret == 0) {
        srand(seed);
    } else {
        srand(time(NULL));
    }

    header = (Poker_Msg_Header *)malloc(sizeof(Poker_Msg_Header));
    if (header == NULL) {
        return NULL;
    }
    memset(header, 0, sizeof(Poker_Msg_Header));
    header->clientSN = clientSN;
    header->serialCode = rand();
    header->msgTotalNum = 1;

    body = (Poker_Msg_Body *)malloc(sizeof(Poker_Msg_Body));
    if (body == NULL) {
        goto head_error;
    }
    memset(body, 0, sizeof(Poker_Msg_Body));
    int infoLen  = strlen(info);
    int actionLen = strlen(action);
    body->msgValue = (char *)malloc(infoLen + 1);
    if (body->msgValue == NULL) {
        goto msg_error;
    }
    body->msgType = (char *)malloc(actionLen + 1);
    if (body->msgType == NULL) {
        goto msg_error;
    }
    memset(body->msgType, 0, actionLen + 1);
    memset(body->msgValue, 0, infoLen + 1);
    strncpy(body->msgType, action, actionLen);
    strncpy(body->msgValue, info, infoLen);

    tail = (Poker_Msg_Tail *)malloc(sizeof(Poker_Msg_Tail));
    if (tail == NULL) {
        goto msg_error;
    }
    memset(tail, 0, sizeof(Poker_Msg_Tail));
    tail->timestamp = time(NULL);
    strncpy(tail->endTag, POKER_MSG_END_TAG, POKER_MSG_END_TAG_LEN);
    module = (Poker_Msg_Module *)malloc(sizeof(Poker_Msg_Module));
    if (module == NULL) {
        goto tail_error;
    }
    memset(module, 0, sizeof(Poker_Msg_Module));
    module->msgHeader = header;
    module->msgBody = body;
    module->msgTail = tail;

    return module;

tail_error:
    if (tail) {
        free(tail);
        tail = NULL;
    }
msg_error:
    if (body && body->msgType) {
        free(body->msgType);
        body->msgType = NULL;
    }
    if (body && body->msgValue) {
        free(body->msgValue);
        body->msgValue = NULL;
    }
body_error:
    if(body) {
        free(body);
        body = NULL;
    }
head_error:
    if (header) {
        free(header);
        header = NULL;
    }
    return NULL;
}

void free_poker_msg_module(Poker_Msg_Module *module)
{
    if (module) {
        if (module->msgHeader) {
            free(module->msgHeader);
            module->msgHeader = NULL;
        }
        if (module->msgBody) {
            if (module->msgBody->msgType) {
                free(module->msgBody->msgType);
                module->msgBody->msgType = NULL;
            }
            if (module->msgBody->msgValue) {
                free(module->msgBody->msgValue);
                module->msgBody->msgValue = NULL;
            }
            free(module->msgBody);
            module->msgBody = NULL;
        }
        if (module->msgTail) {
            free(module->msgTail);
            module->msgTail = NULL;
        }
        free(module);
        module = NULL;
    }
}

char * build_poker_msg(int clientSN, const char *action, const char *info)
{
    char *buf = NULL;

    if (action == NULL || info == NULL) {
        return NULL;
    }
    Poker_Msg_Module *module = build_poker_msg_module(clientSN, action, info);
    if (module == NULL) {
        return NULL;
    }

    cJSON *json_module = build_poker_json_msg(module);
    if (json_module == NULL) {
        goto err_json_module;
    }
    char *json_info = cJSON_PrintUnformatted(json_module);
    if (json_info == NULL) {
        goto err_free_json;
    }
    Poker_Msg_STU msg;
    strncpy(msg.magic, POKER_MSG_MAGIC, POKER_MSG_MAGIC_LEN);
    msg.magic[POKER_MSG_MAGIC_LEN] = '\0';
    msg.len = strlen(json_info);
    msg.info = json_info;

    //int len = strlen(msg.magic) + sizeof(msg.len) + msg.len + 1 + 1; // 加上'\n'和'\0'
    int len = POKER_MSG_MAGIC_LEN + sizeof(msg.len) + msg.len + 1 + 1; // 加上'\n'和'\0'
    /*  magic[8] + len[4] + json_info(156,不含有\n ) + \n + \0  */
    buf = (char *)malloc(len);
    if (buf) {
        memset(buf, 0, len);
        snprintf(buf, len, "%s%04x%s", msg.magic, msg.len, msg.info);
        buf[strlen(buf)] = '\n';
    }

err_free_json:
    if (json_info) {
        free(json_info);
        json_info = NULL;
    }
err_json_module:
    if (json_module) {
        cJSON_Delete(json_module);
        json_module = NULL;
    }
err_free_module:
    if (module) {
        printf("[%s]\n",__FUNCTION__);
        free_poker_msg_module(module);
        module = NULL;
    }
    printf("[%s]buf=%s\n", __FUNCTION__, buf);
    return buf;
}


int checkMsgMagic(char *buf)
{
    if (0 == strncmp(buf, POKER_MSG_MAGIC, POKER_MSG_MAGIC_LEN)) {
        return 0;
    }
    return -1;
}

int checkMsgTailTag(char *buf)
{
    if (0 == strncmp(buf, POKER_MSG_END_TAG, POKER_MSG_END_TAG_LEN)) {
        return 0;
    }
    return -1;
}

int parseMsgHeader(char *buf, int *len)
{
    char bufLen[5] = {0};
    int ret = -1;
    if (len == NULL || buf == NULL) {
        return ret;
    }
    printf("=======[%s] start ========\n", __FUNCTION__);
    printf("[%s] buf=%s\n", __FUNCTION__, buf);
    printf("=======[%s] end ========\n", __FUNCTION__);
    if (checkMsgMagic(buf) == 0) {
        strncpy(bufLen, buf + POKER_MSG_MAGIC_LEN, sizeof(bufLen)-1);
        printf("%s\n", bufLen);
        // 16进制,+1的原因是\n未统计在内
        *len = 1+strtol(bufLen, NULL, 16);
        ret =  0;
    } else {
        ret = -2;
    }
    printf("[%s]ret = %d\n", __FUNCTION__, ret);
    return ret;
}

Poker_Msg_Module * parseJsonMsg(char *json_info, int json_len)
{
    // printf("%s\n",jsonInfo);
    int i = 0;
    Poker_Msg_Module *module = NULL;
    Poker_Msg_Body *body = NULL;
    Poker_Msg_Header *header = NULL;
    Poker_Msg_Tail *tail = NULL;
    cJSON *json_module = NULL;
    json_module = cJSON_ParseWithLength(json_info, json_len);
    if(json_module == NULL) {
        return NULL;
    }
    header = (Poker_Msg_Header *)malloc(sizeof(Poker_Msg_Header));
    if (header) {
        memset(header, 0, sizeof(Poker_Msg_Header));
    } else {
        goto error_json_module;
    }

    cJSON *header_SN = cJSON_GetObjectItem(json_module, POKER_MSG_HEADER_CLIENT_SN);
    if (cJSON_IsNumber(header_SN)) {
        printf("clientSN:%d\n", header_SN->valueint);
        header->clientSN = header_SN->valueint;
    }
    cJSON *header_code = cJSON_GetObjectItem(json_module, POKER_MSG_HEADER_SERIAL_CODE);
    if (cJSON_IsNumber(header_code)) {
        printf("serialCode:%d\n", header_code->valueint);
        header->serialCode = header_code->valueint;
    }

    cJSON *header_num = cJSON_GetObjectItem(json_module, POKER_MSG_HEADER_TOTAL_NUM);
    if (cJSON_IsNumber(header_num)) {
        printf("msgTotalNum:%d\n", header_num->valueint);
        header->msgTotalNum = header_num->valueint;
        if (header_num->valueint > MAX_POKER_MSG_NUM) {
            printf("error, msgTotalNum overload:%d \n", header_num->valueint);
        }
    }
    cJSON *json_body = cJSON_GetObjectItem(json_module, POKER_MSG_BODY_TAG);
    if (cJSON_IsArray(json_body)) {
        int size = cJSON_GetArraySize(json_body);
        printf("arraySize = %d\n", size);
        if (size != header_num->valueint) {
            printf("msg size is not equal.\n");
        }
        body = (Poker_Msg_Body *)malloc(sizeof(Poker_Msg_Body) * size);
        if (body) {
            memset(body, 0, sizeof(Poker_Msg_Body) * size);
        } else {
            goto error_head;
        }

        for (i = 0; i < size; i++) {
            cJSON *ary = cJSON_GetArrayItem(json_body, i);
            if (cJSON_IsObject(ary)) {
                cJSON *type = cJSON_GetObjectItem(ary, POKER_MSG_BODY_TYPE);
                if (cJSON_IsString(type)) {
                    printf("type=%s\n", type->valuestring);
                    int body_type_len = (int)strlen(type->valuestring);
                    char * body_type = (char *)malloc(body_type_len + 1);
                    if (body_type) {
                        memset(body_type, 0, body_type_len+1);
                        strncpy(body_type, type->valuestring, body_type_len);
                        body[i].msgType = body_type;
                    }
                }
                cJSON *value = cJSON_GetObjectItem(ary, POKER_MSG_BODY_VALUE);
                if (cJSON_IsString(value)) {
                    printf("value=%s\n", value->valuestring);
                    int body_val_len = (int)strlen(value->valuestring);
                    char *body_value = (char *)malloc(body_val_len+1);
                    if (body_value) {
                        memset(body_value, 0, body_val_len + 1);
                        strncpy(body_value, value->valuestring, body_val_len);
                        body[i].msgValue = body_value;
                    }
                }
            }

        }
    }
    tail = (Poker_Msg_Tail *)malloc(sizeof(Poker_Msg_Tail));
    if (tail) {
        memset(tail, 0, sizeof(Poker_Msg_Tail));
    } else {
        goto error_body;
    }

    cJSON *stamp = cJSON_GetObjectItem(json_module, POKER_MSG_TAIL_TIMESTAMP);
    if (cJSON_IsNumber(stamp)) {
        printf("timeStamp=%d\n", stamp->valueint);
        tail->timestamp = stamp->valueint;
    }

    cJSON *end = cJSON_GetObjectItem(json_module, POKER_MSG_TAIL_END_TAG);
    if (cJSON_IsString(end) && 0 == checkMsgTailTag(end->valuestring)) {
        printf("endTag=%s\n", end->valuestring);
        strncpy(tail->endTag, end->valuestring, sizeof(tail->endTag)-1);
    }
    module = (Poker_Msg_Module *)malloc(sizeof(Poker_Msg_Module));
    if (module) {
        memset(module, 0, sizeof(Poker_Msg_Module));
        module->msgHeader = header;
        module->msgBody = body;
        module->msgTail = tail;
    } else {
        goto error_tail;
    }

error_json_module:
    if (json_module) {
        cJSON_Delete(json_module);
        json_module = NULL;
    }
    return module;

error_tail:
    if (tail) {
        free(tail);
        tail = NULL;
    }

error_body:
    if (body) {
        free(body);
        body = NULL;
    }

error_head:
    if (header) {
        free(header);
        header = NULL;
    }
    return NULL;
}

int getRandIntByDev(unsigned int *data)
{
    int ret = 0, rand = 0;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    int result = read(fd, &rand, sizeof(rand));
    if (result < 0) {
        ret = -1;
    } else {
        *data = (unsigned int)rand;
    }

    close(fd);
    return ret;
}