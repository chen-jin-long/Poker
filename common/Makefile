TARGET = common.o
all : $(TARGET)
$(TARGET) : common.c
	gcc -c $< -o $@ -I $(ROOT_DIR)/poker/include -I $(ROOT_DIR)/cJSON/

.PHONY : all
