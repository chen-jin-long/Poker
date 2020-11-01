ROOT_DIR = $(shell pwd)
#SUB_DIR = $(shell (ls -l | grep ^d | awk '{if($9 != "cJSON") print $9}'))
#SUB_DIR := $(shell (ls -l | grep ^d | awk '{print $9}'))
#SUB_DIR := $(wildcard */.)
SUB_DIR := common  poker game server  client
CC = gcc
CFLAGS := -g -O
MAKE = $(CC) $(CFLAGS)
export ROOT_DIR
export CC
export CFLAGS
export MAKE

SOURCE := $(wildcard *.c) $(wildcard *.cc) 
OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SOURCE))) 
mycompile :=
server: common poker game
subdir:
	@for dir in $(SUB_DIR); \
	do \
		if [ $$dir != "cJSON/." ]; then \
			echo $$dir; \
			make -C $$dir || exit 1; \
		fi \
		  #echo $$dir; \
	done

define TEST/compile
	$(mycopmile):
		echo "print mycompile"

endef	

#为什么这里不能用all: $subdir

all: subdir
	$(call TEST/compile)
	@echo "build end..."

clean:
	@for dir in $(SUB_DIR); \
	do \
		make -C $$dir clean || exit 1; \
	done
#export ROOT_DIR 放在这个位置也没关系

.PHONY:all subdir clean

