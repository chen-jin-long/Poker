TARGET = client
all: $(TARGET)
CTAGS = -DAUTO_CLIENT -g
LDFLAGS = -Wl,-rpath=$(ROOT_DIR)/cJSON
#CTAGS = -g
$(TARGET):client.c $(ROOT_DIR)/cJSON/libmsgJson.so
	gcc $< -o $@  -lpthread -I $(ROOT_DIR)/poker/include -I $(ROOT_DIR)/common -I $(ROOT_DIR)/cJSON/ $(ROOT_DIR)/common/common.o $(CTAGS) $(LDFLAGS) -L $(ROOT_DIR)/cJSON -lmsgJson
clean:
	rm $(TARGET)

.PHONY:all clean
