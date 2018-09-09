CC = gcc
#CFLAGS += -I./include
CFLAGS += -L -lpoker
EXEC = poker_main
#OBJS = Utils.o
SRCS = poker_main.c

all : $(EXEC)

.PHONY : all

Utils.o : Utils.c
	$(CC) -c Utils.c -I./include/

poker.o : poker.c
	$(CC) -c poker.c -I./include/
poker_send.o : poker_send.c
	$(CC) -c poker_send.c poker.c Utils.c -I./include/ 
libpoker.a : Utils.o poker.o poker_send.o
	ar -rv libpoker.a Utils.o poker.o poker_send.o

poker_main : $(SRCS) libpoker.a
	$(CC) -I./include $(CFAGS) $(SRCS) libpoker.a -o $@


clean :
	rm $(EXEC)
	rm *.o
	rm *.a
.PHONY : clean
