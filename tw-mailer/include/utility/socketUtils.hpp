#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libraries/spdlog/spdlog.h"

#include <iostream>
#include <string>

#define RESPONSE_ERR "ERR\n"
#define RESPONSE_OK "OK\n"

bool isOpen(int socketFileDescriptor)
{
    char buffer;
    int result = recv(socketFileDescriptor, &buffer, 1, MSG_PEEK | MSG_WAITALL);
    return (result > 0);
}

std::string readAll(int socketFileDescriptor, int bufferSize)
{
    std::string messageText;

    char buffer[bufferSize];
    int messageLength;
    do
    {
        messageLength = read(socketFileDescriptor, buffer, sizeof(buffer) - 1);
        if (messageLength < 0)
            return "";
        buffer[messageLength] = '\0';
        messageText += std::string(buffer);
    } while (messageLength >= bufferSize - 1);

    spdlog::debug("Read {}", std::string(messageText));

    return messageText;
}

void writeAll(int socketFileDescriptor, const std::string &text, bool logging = false)
{
    if (send(socketFileDescriptor, text.c_str(), strlen(text.c_str()) + 1, MSG_NOSIGNAL) < 0 && logging)
    {
        spdlog::error("Could not write string {}!", text);
    }
}