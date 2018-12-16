TARGET = server
POKER_GAME=../game/poker_game.o
all: $(TARGET)
$(TARGET):$(POKER_GAME) poker_server_pthread.cpp poker_server.h 
	gcc -o server poker_server_pthread.cpp $(POKER_GAME) -I ../game -I ../poker/include -I ../common -lpthread -L ../poker -lpoker ../common/common.o -g
clean:
	rm server

.PHONY: all clean
