SRCS:=$(wildcard *.cpp)
EFLS:=$(patsubst %.cpp,%,$(SRCS))
CC:=g++
CFLAGS:=-Werror -pthread -lssl -lcrypto -lcurl
 
all:$(EFLS)
	echo $(EFLS)
%:%.cpp
	$(CC) $< -o $@ $(CFLAGS)
clean:
	rm -rf $(EFLS)

