TARGET = client
all: $(TARGET)
CTAGS = -DAUTO_CLIENT -g
#CTAGS = -g
$(TARGET):client.c
	gcc $< -o $@  -lpthread -I ../poker/include -I ../common ../common/common.o $(CTAGS)
clean:
	rm $(TARGET)

.PHONY:all clean
