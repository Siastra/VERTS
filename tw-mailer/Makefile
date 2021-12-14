CC = g++
CFLAGS = -std=c++2a -o
PTHREAD = -lpthread
SERVER_DIR = src/server
CLIENT_DIR = src/client

all: client server

server: $(SERVER_DIR)/Command.cpp $(SERVER_DIR)/main.cpp $(SERVER_DIR)/Server.cpp 
	$(CC) $(CFLAGS) ./build/twmailer-server $(SERVER_DIR)/Command.cpp $(SERVER_DIR)/main.cpp $(SERVER_DIR)/Server.cpp -I include $(PTHREAD)

client: $(CLIENT_DIR)/client.cpp
	$(CC) $(CFLAGS) ./build/twmailer-client $(CLIENT_DIR)/client.cpp -I include

clean:
	rm -f ./build/twmailer-client ./build/twmailer-server