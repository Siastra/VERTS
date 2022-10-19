CC = g++
CFLAGS = -std=c++2a -o
PTHREAD = -lpthread
LDAP = -lldap -llber
SERVER_DIR = src/server
CLIENT_DIR = src/client

all: client server

server: $(SERVER_DIR)/Command.cpp $(SERVER_DIR)/main.cpp $(SERVER_DIR)/Server.cpp $(SERVER_DIR)/ServerCommunication.cpp $(SERVER_DIR)/Mail.cpp 
	$(CC) $(CFLAGS) ./build/twmailer-server $(SERVER_DIR)/Command.cpp $(SERVER_DIR)/main.cpp $(SERVER_DIR)/Server.cpp $(SERVER_DIR)/ServerCommunication.cpp $(SERVER_DIR)/Mail.cpp -I include $(PTHREAD) $(LDAP)

client: $(CLIENT_DIR)/client.cpp
	$(CC) $(CFLAGS) ./build/twmailer-client $(CLIENT_DIR)/client.cpp -I include

clean:
	rm -f ./build/twmailer-client ./build/twmailer-server