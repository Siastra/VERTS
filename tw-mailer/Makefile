CC = g++
CFLAGS = -std=c++2a -o
PTHREAD = -lpthread

all: client server

server: src/server.cpp
	$(CC) $(CFLAGS) ./build/server src/server.cpp -I include $(PTHREAD)

client: src/client.cpp
	$(CC) $(CFLAGS) ./build/client src/client.cpp -I include

clean:
	rm -f ./build/client ./build/server