TARGET = client
all: $(TARGET)
$(TARGET):client.c
	gcc $< -o $@  -lpthread -I ../poker/include -I ../common ../common/common.o -g
clean:
	rm $(TARGET)

.PHONY:all clean
