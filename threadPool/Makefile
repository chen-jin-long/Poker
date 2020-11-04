TARGET = libthreadPool.a
all: $(TARGET)
CTAGS = -g
SOURCE := $(wildcard *.c) $(wildcard *.cc) 
OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SOURCE)))
$(TARGET):$(OBJS)
	gcc $(OBJS) -lpthread -I . $(CTAGS)
	ar -rv $@ thread_pool.o queue.o

clean:
	rm *.o
	rm $(TARGET)

.PHONY:all clean
