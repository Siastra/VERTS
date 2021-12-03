#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "libraries/CLI/CLI.hpp"
#include "libraries/spdlog/spdlog.h"

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
    socketAddress.sin_port = htons(port);
    inet_aton(address, &socketAddress.sin_addr);

    if (connect(socketFileDescriptor, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) != 0)
    {
        spdlog::error("Unable to connect to server {}!", inet_ntoa(socketAddress.sin_addr));
        close(socketFileDescriptor);
        return EXIT_FAILURE;
    }

    spdlog::info("Connection with server {} established", inet_ntoa(socketAddress.sin_addr));
    return socketFileDescriptor;
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
        std::cout << "Please select your action: 1 - SEND, 2 - READ, 3 - LIST, 4 - DEL, 5- QUIT\n";
        std::cout << ">>";
        std::cin >> selection;
        switch (selection)
        {
        case 1:
            spdlog::info("SEND to be implemented...");
            break;
        case 2:
            spdlog::info("READ to be implemented...");
            break;
        case 3:
            spdlog::info("LIST to be implemented...");
            break;
        case 4:
            spdlog::info("DEL to be implemented...");
            break;
        case 5:
            spdlog::info("Quitting application...");
            abort = true;
            break;
        default:
            spdlog::warn("Unknown argument!");
            break;
        }
    }

    close(socketFileDescriptor);
    return EXIT_SUCCESS;
}
