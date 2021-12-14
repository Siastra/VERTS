#include "Server.hpp"
#include "Command.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>

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

    while (!mAbortRequested)
    {
        int newSocket{ -1 };
        std::cout << "Waiting for connections...\n";
        mAddressLength = sizeof(struct sockaddr_in);
        if (!Accept(newSocket))
            break;

        std::cout << "Client connected from " << inet_ntoa(mClientAddress.sin_addr) << ":" << ntohs(mClientAddress.sin_port) << "\n";
        ClientCommunication(newSocket);
    }
}

void Server::ClientCommunication(int& communicationSocket)
{
    int size;
    char buffer[BUFFER];
    SendData(communicationSocket, "CONNECTED\n");
    do
    {
        size = recv(communicationSocket, buffer, BUFFER - 1, 0);
        if (size == -1)
        {
            if (mAbortRequested)
            {
                perror("recv error after aborted");
            }
            else
            {
                perror("recv error");
            }
            break;
        }

        if (size == 0)
        {
            printf("Client closed remote socket\n"); // ignore error
            break;
        }

        //Isolate command
        std::string message = buffer;
        const size_t commandEnd = message.find('\n');
        const std::string command = message.substr(0, commandEnd);
        message.erase(0, commandEnd + 1);


        //Perform action depening on command
        if (command == "SEND")
        {
            if (Command::Send(message))
                SendData(communicationSocket, "OK\n");
            else
                SendData(communicationSocket, "ERR\n");
        }
        else if (command == "READ")
        {
            const std::string content = Command::Read(message);
            std::cout << content << std::endl;
            if (!content.empty())
                SendData(communicationSocket, "OK\n" + content);
            else
                SendData(communicationSocket, "ERR\n");

        }
        else if (command == "LIST")
        {
            std::vector<std::string> mails = Command::List(message);
            SendData(communicationSocket, std::to_string(mails.size()) + '\n');
            if (mails.size() > 0)
            {
                std::string completeMessage{ "" };
                for (const auto& mail : mails)
                    completeMessage += mail + '\n';
                SendData(communicationSocket, completeMessage);
            }
        }
        else if (command == "DEL")
        {
            if (Command::Delete(message))
                SendData(communicationSocket, "OK\n");
            else
                SendData(communicationSocket, "ERR\n");
        }

    } while (!Command::Quit(buffer) && !mAbortRequested);

    // closes/frees the descriptor if not already
    if (communicationSocket != -1)
    {
        if (shutdown(communicationSocket, SHUT_RDWR) == -1)
            perror("shutdown new_socket");
        if (close(communicationSocket) == -1)
            perror("close new_socket");
        communicationSocket = -1;
    }
}

void Server::SendData(int& communicationSocket, const std::string& data)
{
    if (send(communicationSocket, data.c_str(), data.length(), 0) == -1)
    {
        perror("send failed");
        exit(EXIT_FAILURE);
    }
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