CC = g++
#GTEST_DIR = /home/cgd/googletest/googletest/include
CFLAGS += -I../include
CFLAGS += -I../../common
CFLAGS += -I../../game/
CFLAGS += -std=c++11
CFLAGS += -pthread

all: poker_ut

.PHONY : all

poker_ut : poker_test.cc
	$(CC) $(CFLAGS)  poker_test.cc -L.. -lpoker -lgtest ../../common/common.o -o $@ -g

clean :
	rm poker_ut

.PHONY : clean

