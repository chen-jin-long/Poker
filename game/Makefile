TARGET = poker_game.o
all: $(TARGET)
$(TARGET):poker_game.c poker_game.h
	gcc -c  poker_game.c -I $(ROOT_DIR)/poker/include/ -I $(ROOT_DIR)/common/ -I $(ROOT_DIR)/cJSON/
clean:
	rm $(TARGET)

.PHONY: all clean
