#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <initializer_list>

enum class SocketOption
{
    ReuseAddress,
    ReusePort
};

class Server
{
public:
    Server(const int port, const std::string& mailDirectory);

    void CreateSocket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
    void Run();

private:
    bool Accept(int& communicationSocket);
    void Bind();
    void StartListening();
    void SetOptions(std::initializer_list<SocketOption> options);

private:
    static constexpr int MAX_WAITING_CONNECTIONS = 5;
    bool mAbortRequested;

    int mPort;
    int mSocket;
    socklen_t mAddressLength;
    struct sockaddr_in mAddress;
    struct sockaddr_in mClientAddress;
    pid_t mPid;
};