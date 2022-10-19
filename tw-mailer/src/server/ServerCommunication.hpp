#pragma once
#include <string>
#include "Command.hpp"

class ServerCommunication
{
public:
    ServerCommunication(int communicationSocket, const std::string sessionAddress);
    ~ServerCommunication();

private:
    void ClientCommunication();
    void SendData(const std::string& data);
    void ClearSession();

private:
    static constexpr int BUFFER = 1024;
    int mSocket;
    Command mCmd;
};