TARGET = poker_game.o
all: $(TARGET)
$(TARGET):poker_game.c poker_game.h $(ROOT_DIR)/poker/include/Utils.h $(ROOT_DIR)/poker/Utils.o
	gcc -c  poker_game.c $(ROOT_DIR)/poker/Utils.o -I $(ROOT_DIR)/poker/include/ -I $(ROOT_DIR)/common/ -I $(ROOT_DIR)/cJSON/
clean:
	rm $(TARGET)

.PHONY: all clean
