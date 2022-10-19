#include "Server.hpp"
#include "ServerCommunication.hpp"
#include "Command.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>

//SIGNAL handling not implemented yet 

Server::Server(const int port, const std::string& mailDirectory)
    : mSocket(-1), mAddressLength(-1), mAbortRequested(false)
{
    mPort = port;
    Command::SetMailDirectory(mailDirectory);
    mAddress = {};
    mClientAddress = {};
}

void Server::CreateSocket(int domain, int type, int protocol)
{
    if ((mSocket = socket(domain, type, protocol)) == -1)
    {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }
}

void Server::Bind()
{
    mAddress.sin_family = AF_INET;
    mAddress.sin_addr.s_addr = INADDR_ANY;
    mAddress.sin_port = htons(mPort);

    if (bind(mSocket, reinterpret_cast<struct sockaddr*>(&mAddress), sizeof(mAddress)) == -1)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
}

void Server::StartListening()
{
    if (listen(mSocket, MAX_WAITING_CONNECTIONS) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
}

bool Server::Accept(int& communicationSocket)
{
    if ((communicationSocket = accept(mSocket, reinterpret_cast<struct sockaddr*>(&mClientAddress), &mAddressLength)) == -1)
    {
        mAbortRequested ? perror("accept error after arbort") : perror("accept error");
        return false;
    }
    return true;
}

void Server::Run()
{
    Bind();
    SetOptions({ SocketOption::ReuseAddress, SocketOption::ReusePort });
    StartListening();
    // pid_t wpid;
    // int status = 0;

    while (!mAbortRequested)
    {
        int communicationSocket{ -1 };
        std::cout << "Waiting for connections...\n";
        mAddressLength = sizeof(struct sockaddr_in);
        if (!Accept(communicationSocket))
            break;

        auto handleClient = [this, communicationSocket]() {
            std::cout << "Client connected from " << inet_ntoa(mClientAddress.sin_addr) << ":" << ntohs(mClientAddress.sin_port) << "\n";
            ServerCommunication com(communicationSocket, std::string(inet_ntoa(mClientAddress.sin_addr)));
            std::cout << "Client disconnected from " << inet_ntoa(mClientAddress.sin_addr) << ":" << ntohs(mClientAddress.sin_port) << "\n";
            close(communicationSocket);
        };

        auto clientThread = std::make_unique<std::thread>(handleClient);
        clientThread.get()->detach();

        // if ((mPid = fork()) == 0)
        // {
        //     std::cout << "Client connected from " << inet_ntoa(mClientAddress.sin_addr) << ":" << ntohs(mClientAddress.sin_port) << "\n";
        //     ServerCommunication com(communicationSocket, std::string(inet_ntoa(mClientAddress.sin_addr)));
        //     std::cout << "Client disconnected from " << inet_ntoa(mClientAddress.sin_addr) << ":" << ntohs(mClientAddress.sin_port) << "\n";
        //     close(communicationSocket);
        //     exit(0);
        // }
        // close(communicationSocket);
    }
    // wait for all child process to finish
    // while ((wpid = wait(&status)) > 0);
}

void Server::SetOptions(std::initializer_list<SocketOption> options)
{
    int reuseValue{ 1 };

    for (auto option : options)
        switch (option)
        {
        case SocketOption::ReuseAddress:
            if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1)
            {
                perror("set socket options - reuseAddr");
                exit(EXIT_FAILURE);
            }
            break;
        case SocketOption::ReusePort:
            if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1)
            {
                perror("set socket options - reusePort");
                exit(EXIT_FAILURE);
            }
            break;
        }
}