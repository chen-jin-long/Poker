CC = gcc
#CFLAGS += -I./include
CFLAGS += -g -L -lpoker
EXEC = poker_main
#OBJS = Utils.o
SRCS = poker_main.c

all : $(EXEC)

.PHONY : all

Utils.o : Utils.c
	$(CC) -c Utils.c -I./include/

poker.o : poker.c
	$(CC) $(CFLAGS) -c poker.c -I./include/ 
poker_send.o : poker_send.c
	$(CC) $(CFLAGS) -c poker_send.c poker.c Utils.c -I./include/ -I$(ROOT_DIR)/game/ -DFIXED_POKER
libpoker.a : Utils.o poker.o poker_send.o
	ar -rv libpoker.a Utils.o poker.o poker_send.o

poker_main : $(SRCS) libpoker.a
	$(CC) -I./include $(CFLAGS) $(SRCS) libpoker.a -o $@


clean :
	rm $(EXEC)
	rm *.o
	rm *.a
.PHONY : clean
