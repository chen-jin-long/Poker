TARGET = msg_json.so
TOOL_BIN = tool
CFLAGS += -g
all : $(TARGET)
$(TARGET) : cJSON.c msg_json.c
	gcc -c  -fpic msg_json.c cJSON.c $(CFLAGS)
	gcc -shared  msg_json.o cJSON.o $(CFLAGS) -o libmsgJson.so 
# cJson_tools test bin
#gcc -o a.out cJson_tools.c cJSON.o  msg_json.o
$(TOOL_BIN): $(TARGET) cJSON.c msg_json.c cJson_tools.c
	gcc -o $@ cJson_tools.c cJSON.o  msg_json.o $(CFLAGS)

clean :
	rm *.o
	rm $(TOOL_BIN)

.PHONY : all clean 
