#include "ServerCommunication.hpp"
#include "Command.hpp"

#include <sys/socket.h>
#include <ldap.h>
#include <unistd.h>
#include <iostream>
#include <mutex>

ServerCommunication::ServerCommunication(int communicationSocket, const std::string sessionAddress)
    :mSocket(communicationSocket), mCmd()
{
    mCmd.SetSessionAddress(sessionAddress);
    ClientCommunication();
}

ServerCommunication::~ServerCommunication()
{
    ClearSession();
}

void ServerCommunication::ClientCommunication()
{
    int size;
    char buffer[BUFFER];
    do
    {
        std::string message{};
        while ((size = recv(mSocket, buffer, BUFFER - 1, 0)) > 0)
        {
            message.append(buffer);
            if (message.ends_with("\n") && size < BUFFER)
                break;
        }
        if (size == -1)
        {
            perror("recv error");
            break;
        }

        if (size == 0)
        {
            printf("Client closed remote socket\n"); // ignore error
            break;
        }
        //Isolate command
        const size_t commandEnd = message.find('\n');
        const std::string command = message.substr(0, commandEnd);
        message.erase(0, commandEnd + 1);

        //Perform action depending on command
        if (command == "LOGIN")
        {
            switch (mCmd.Login(message))
            {
            case -1:
                SendData("BLOCKED\n");
                break;
            case 0:
                SendData("ERR\n");
                break;
            case 1:
                SendData("OK\n");
                break;
            }
        }
        else if (command == "SEND")
        {
            if (mCmd.Send(message))
                SendData("OK\n");
            else
                SendData("ERR\n");
        }
        else if (command == "READ")
        {
            const std::string content = mCmd.Read(message);
            if (!content.empty())
                SendData("OK\n" + content);
            else
                SendData("ERR\n");

        }
        else if (command == "LIST")
        {
            std::vector<std::string> mails = mCmd.List(message);
            SendData(std::to_string(mails.size()) + '\n');
            if (mails.size() > 0)
            {
                std::string completeMessage{ "" };
                for (const auto& mail : mails)
                    completeMessage += mail + '\n';
                SendData(completeMessage);
            }
        }
        else if (command == "DEL")
        {
            if (mCmd.Delete(message))
                SendData("OK\n");
            else
                SendData("ERR\n");
        }

    } while (!mCmd.Quit(buffer));

    // closes/frees the descriptor if not already
    if (mSocket != -1)
    {
        if (shutdown(mSocket, SHUT_RDWR) == -1)
            perror("shutdown new_socket");
        if (close(mSocket) == -1)
            perror("close new_socket");
        mSocket = -1;
    }
}

void ServerCommunication::SendData(const std::string& data)
{
    if (send(mSocket, data.c_str(), data.length(), 0) == -1)
    {
        perror("send failed");
        exit(EXIT_FAILURE);
    }
}

void ServerCommunication::ClearSession()
{
}