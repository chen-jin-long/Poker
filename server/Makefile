TARGET = server
POKER_GAME = $(ROOT_DIR)/game/poker_game.o
all: $(TARGET)
$(TARGET):$(POKER_GAME)  poker_server_pthread.c $(POKER_GAME)  poker_server.h $(ROOT_DIR)/poker/libpoker.a   $(ROOT_DIR)/common/common.o 
	gcc -o server poker_server_pthread.c $(POKER_GAME) -I $(ROOT_DIR)/game -I $(ROOT_DIR)/poker/include -I $(ROOT_DIR)/common -I $(ROOT_DIR)/Utils -lpthread -L $(ROOT_DIR)/poker -lpoker $(ROOT_DIR)/common/common.o -g
clean:
	rm server

.PHONY: all clean
