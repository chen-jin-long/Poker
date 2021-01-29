TARGET = libgame.a
#$(POKER_GAME): poker_game.c poker_game.h
#	gcc -c  poker_game.c -I $(ROOT_DIR)/poker/include/ -I $(ROOT_DIR)/common/ -I $(ROOT_DIR)/cJSON/

libgame.a : poker_game.c $(ROOT_DIR)/poker/Utils.c $(ROOT_DIR)/poker/poker_send.c
	gcc -c poker_game.c  -I $(ROOT_DIR)/poker/include/ -I $(ROOT_DIR)/common/  -I$(ROOT_DIR)/game/ -I$(ROOT_DIR)/cJSON/
	ar -rcs libgame.a  poker_game.o $(ROOT_DIR)/poker/Utils.o $(ROOT_DIR)/poker/poker_send.o 

all: $(TARGET)


clean:
	-rm *.o
	rm $(TARGET)

.PHONY: all clean
