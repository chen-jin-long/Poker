CC = g++
#GTEST_DIR = /home/cgd/googletest/googletest/include
CFLAGS += -I../include
CFLAGS += -I../../common
CFLAGS += -I../../game/
CFLAGS += -std=c++11
CFLAGS += -pthread

all: poker_ut

.PHONY : all

poker_ut : poker_test.cc ../libpoker.a
	$(CC) $(CFLAGS)  poker_test.cc ../libpoker.a -lgtest $(ROOT_DIR)/common/common.o -o $@ -g

clean :
	rm poker_ut

.PHONY : clean

