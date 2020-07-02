CC = g++
GTEST_DIR = /home/cgd/googletest/googletest/include
CFLAGS += -isystem $(GTEST_DIR)
CFLAGS += -I../include
CFLAGS += -I../../common
CFLAGS += -std=c++11
CFLAGS += -pthread

all: poker_ut

.PHONY : all

poker_ut : poker_test.cc ../libpoker.a libgtest.a
	$(CC) $(CFLAGS)  poker_test.cc ../libpoker.a libgtest.a ../../common/common.o -o $@ -g

clean :
	rm poker_ut

.PHONY : clean

