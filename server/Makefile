TARGET = server
POKER_GAME = $(ROOT_DIR)/game/poker_game.o
LDFLAGS = -Wl,-rpath=$(ROOT_DIR)/cJSON
all: $(TARGET)
$(TARGET):$(POKER_GAME)  poker_server_pthread.c $(POKER_GAME)  poker_server.h $(ROOT_DIR)/poker/libpoker.a   $(ROOT_DIR)/common/common.o  $(ROOT_DIR)/threadPool/libthreadPool.a $(ROOT_DIR)/cJSON/msg_json.o
	gcc -o server poker_server_pthread.c $(POKER_GAME) $(LDFLAGS) -I $(ROOT_DIR)/game -I $(ROOT_DIR)/poker/include -I $(ROOT_DIR)/common -I $(ROOT_DIR)/Utils -I $(ROOT_DIR)/threadPool -I $(ROOT_DIR)/cJSON -lpthread -L $(ROOT_DIR)/threadPool -lthreadPool -L $(ROOT_DIR)/poker -lpoker $(ROOT_DIR)/common/common.o -L $(ROOT_DIR)/cJSON -lmsgJson -g
clean:
	rm server

.PHONY: all clean
