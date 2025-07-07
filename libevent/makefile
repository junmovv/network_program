# 1. 统一使用追加方式定义INC
LIBEVENT_DIR = /home/vv/libevent/libevent-2.1.12-stable/build
SOURCE_INC = /home/vv/libevent/libevent-2.1.12-stable/include
BUILD_INC = $(LIBEVENT_DIR)/include

# 2. 按需添加路径（避免重复）
INC_DIRS := 
INC_DIRS += -I$(SOURCE_INC)          # 源码头文件
INC_DIRS += -I$(SOURCE_INC)/event2   # 源码event2子目录
INC_DIRS += -I$(BUILD_INC)           # 构建生成的头文件（关键！）

# 3. 其他变量保持不变
CC = g++
CFLAGS = -Wall -g $(INC_DIRS)        # 合并所有-I路径
# LDFLAGS = -L$(LIBEVENT_DIR)/lib -levent -lpthread
# LDFLAGS = -L$(LIBEVENT_DIR)/lib -Wl,-Bstatic -levent -Wl,-Bdynamic -lpthread
LDFLAGS = -levent -lpthread

TARGET = event
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	LD_LIBRARY_PATH=$(LIBEVENT_DIR)/lib ./$(TARGET) 

.PHONY: all clean run