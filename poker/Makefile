CC = gcc
#CFLAGS += -I./include
CFLAGS += -g -L -lpoker
INCLUDE_DIR = $(ROOT_DIR)/poker/include/
EXEC = poker_main
#OBJS = Utils.o
SRCS = poker_main.c

all : $(EXEC)

.PHONY : all

Utils.o : Utils.c
	$(CC) -c Utils.c -I$(INCLUDE_DIR)

poker.o : poker.c $(INCLUDE_DIR)/poker.h
	$(CC) $(CFLAGS) -c poker.c -I$(INCLUDE_DIR)
poker_send.o : poker_send.c
	$(CC) $(CFLAGS) -c poker_send.c  -I$(INCLUDE_DIR) -I$(ROOT_DIR)/game/ -DFIXED_POKER
libpoker.a : Utils.o poker.o poker_send.o
	ar -rv libpoker.a Utils.o poker.o poker_send.o

poker_main : $(SRCS) libpoker.a
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) $(SRCS) libpoker.a -o $@


clean :
	rm $(EXEC)
	rm *.o
	rm *.a
.PHONY : clean
