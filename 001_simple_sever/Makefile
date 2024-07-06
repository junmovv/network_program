SRCS:=$(wildcard *.cpp)
EFLS:=$(patsubst %.cpp,%,$(SRCS))
CC:=g++
CFLAGS:=-Werror -pthread
 
all:$(EFLS)
	echo $(EFLS)
%:%.cpp
	$(CC) $< -o $@ $(CFLAGS)
clean:
	rm -rf $(EFLS)

