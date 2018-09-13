TARGET = client
all: $(TARGET)
$(TARGET):client.c
	gcc $< -o $@  -lpthread -g
clean:
	rm $(TARGET)

.PHONY:all clean
