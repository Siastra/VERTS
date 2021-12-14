/*
 * authors: Benedikt Schottleitner & Marcel Glavanits
 * class: BIF DUAL 3
 * file: socketUtils.hpp
 * date: 13.12.21
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libraries/spdlog/spdlog.h"

#include <iostream>
#include <string>

#define RESPONSE_ERR "ERR\n"
#define RESPONSE_OK "OK\n"

namespace socketUtility
{

    /*
     * Function isOpen for checking, if a specific socket is still open 
     * @param int socketFileDescriptor: the socket file descriptor
     * @return if a specific socket is still open
     */
    bool isOpen(int socketFileDescriptor)
    {
        char buffer;
        int result = recv(socketFileDescriptor, &buffer, 1, MSG_PEEK | MSG_WAITALL);
        return (result > 0);
    }

    /*
     * Function readAll for reading all data, which is received from a specific socket
     * @param int socketFileDescriptor: the socket file descriptor
     * @param int bufferSize: the size of the buffer
     * @return the whole string received on a specific socket
     */
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

    /*
     * Function writeAll for sending given string on a specific socket.
     * @param int socketFileDescriptor: the socket file descriptor
     * @param std::string text the message to send
     */
    void writeAll(int socketFileDescriptor, const std::string &text, bool logging = false)
    {
        if (send(socketFileDescriptor, text.c_str(), strlen(text.c_str()) + 1, MSG_NOSIGNAL) < 0 && logging)
        {
            spdlog::error("Could not write string {}!", text);
        }
    }

}