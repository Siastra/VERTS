#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "libraries/CLI/CLI.hpp"
#include "utility/socketUtils.hpp"


#define BUFFER_SIZE 1024


int connect(const char *address, int port)
{
    auto socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
    {
        spdlog::error("Could not create socket!");
        EXIT_FAILURE;
    }

    struct sockaddr_in socketAddress;

    memset(&socketAddress, 0, sizeof(socketAddress)); // init storage with 0
    socketAddress.sin_family = AF_INET;               // IPv4
    socketAddress.sin_port = htons(port);             //Port
    inet_aton(address, &socketAddress.sin_addr);      //IPv4-address

    if (connect(socketFileDescriptor, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) != 0)
    {
        spdlog::error("Unable to connect to server {}!", inet_ntoa(socketAddress.sin_addr));
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }

    spdlog::debug("Connection with server {} established", inet_ntoa(socketAddress.sin_addr));
    return socketFileDescriptor;
}

std::string receiveResponse(int socketFileDescriptor) {
    std::string response = readAll(socketFileDescriptor, BUFFER_SIZE);
    if (response.empty()) { // The server has to send OK or ERR
        std::cerr << "Server closed the connection!\n";
        exit(EXIT_FAILURE);
    }
    return response;
}

void deleteMessage(int socketFileDescriptor)
{
    std::cout << "Message ID: ";
    std::string messageId;
    getline(std::cin, messageId);
    writeAll(socketFileDescriptor, std::string("DEL\n").append(messageId).append("\n"));
    if (receiveResponse(socketFileDescriptor) == RESPONSE_OK)
    {
        std::cout << "Message deleted successfully!\n";
    }
    else
    {
        std::cout << "There was an error deleting the message.\n";
    }
}

int main(int argc, char const *argv[])
{
    CLI::App app{"TW-Mailer Client"};
    std::string address{"127.0.0.1"};
    int port = {8080};
    bool debug{false};
    int socketFileDescriptor;
    bool abort{false};

    app.add_option("-a,--address", address, "IPv4 Address of the server")->required()->check(CLI::ValidIPV4);
    app.add_option("-p,--port", port, "Port of the server")->required()->check(CLI::PositiveNumber);
    app.add_flag("--debug", debug, "debug messages");

    CLI11_PARSE(app, argc, argv);

    if (debug)
        spdlog::set_level(spdlog::level::debug);

    spdlog::debug("IP address: {}, Port: {}", address, std::to_string(port));

    socketFileDescriptor = connect(address.c_str(), port);

    int selection = 0;
    while (!abort)
    {
        //Input action
        std::cout << "Please select your action: 1 - SEND, 2 - READ, 3 - LIST, 4 - DEL, 5- QUIT\n";
        std::cout << ">>";
        std::cin >> selection;
        std::cin.ignore(INT_MAX, '\n');

        //Respective action is executed
        switch (selection)
        {
        case 1:
            std::cout << "SEND to be implemented...\n";
            break;
        case 2:
            std::cout << "READ to be implemented...\n";
            break;
        case 3:
            std::cout << "LIST to be implemented...\n";
            break;
        case 4:
            deleteMessage(socketFileDescriptor);
            break;
        case 5:
            std::cout << "Quitting application...\n";
            abort = true;
            break;
        default:
            std::cerr << "Unknown argument!\n";
            break;
        }
    }

    close(socketFileDescriptor);
    return EXIT_SUCCESS;
}
