#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <limits>

#include "libraries/CLI/CLI.hpp"
#include "utility/socketUtils.hpp"
#include "utility/stringUtils.hpp"

#define BUFFER_SIZE 1024

using namespace std;

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
    socketAddress.sin_port = htons(port);             // Port
    inet_aton(address, &socketAddress.sin_addr);      // IPv4-address

    if (connect(socketFileDescriptor, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) != 0)
    {
        spdlog::error("Unable to connect to server {}!", inet_ntoa(socketAddress.sin_addr));
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }

    spdlog::debug("Connection with server {} established", inet_ntoa(socketAddress.sin_addr));
    return socketFileDescriptor;
}

string receiveResponse(int socketFileDescriptor)
{
    string response = socketUtility::readAll(socketFileDescriptor, BUFFER_SIZE);
    if (response.empty())
    { // The server has to send OK or ERR
        cerr << "Server closed the connection!\n";
        exit(EXIT_FAILURE);
    }
    return response;
}

int readNumericValue()
{
    int value;
    while (!(cin >> value)) // read line until numeric value is entered
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ignore everything after numeric value
    return value;
}

string readUsername()
{
    string username = "";
    getline(cin, username);

    while (!stringUtility::checkUsername(username))
    {
        cout << "Please enter a valid username(only lower case letters and digits, max. 8 chars): ";
        getline(cin, username);
    }
    return username;
}

void sendMessage(int socketFileDescriptor)
{
    string sender, receiver, subject, content, line;
    cout << "Sender: ";
    sender = readUsername();
    cout << "Receiver: ";
    receiver = readUsername();
    cout << "Subject(max. 80 characters): ";
    getline(cin, subject);
    cout << "Message-Content(ends with dot): " << endl;
    while (line != ".")
    {
        cout << ">>";
        getline(cin, line);
        content.append(line).append("\n");
    }
    socketUtility::writeAll(socketFileDescriptor, string("SEND\n").append(sender).append("\n").append(receiver).append("\n").append(subject).append("\n").append(content));
    if (receiveResponse(socketFileDescriptor) == RESPONSE_OK) {
        cout << "Message sent successfully!\n";
    } else {
        cout << "There occured an error sending the message.\n";
    }
}

void readMessage(int socketFileDescriptor)
{
    cout << "Please enter your username: ";
    string username = readUsername();
    cout << "Message ID: ";
    string messageId = to_string(readNumericValue());

    socketUtility::writeAll(socketFileDescriptor, string("READ\n").append(username).append("\n").append(messageId).append("\n"));
    if (receiveResponse(socketFileDescriptor) == RESPONSE_OK)
    {
        cout << "Message read successfully!\n";
    }
    else
    {
        cerr << "There was an error retrieving the message.\n";
    }
}

void listMessages(int socketFileDescriptor)
{
    cout << "Please enter your username: ";
    string username = readUsername();

    socketUtility::writeAll(socketFileDescriptor, string("LIST\n").append(username).append("\n"));
    if (receiveResponse(socketFileDescriptor) == RESPONSE_OK)
    {
        cout << "Message deleted successfully!\n";
    }
    else
    {
        cerr << "There was an error retrieving the list of messages.\n";
    }
}

void deleteMessage(int socketFileDescriptor)
{
    cout << "Message ID: ";
    int messageId;
    messageId = readNumericValue();
    socketUtility::writeAll(socketFileDescriptor, string("DEL\n").append(to_string(messageId)).append("\n"));
    if (receiveResponse(socketFileDescriptor) == RESPONSE_OK)
    {
        cout << "Message deleted successfully!\n";
    }
    else
    {
        cerr << "There was an error deleting the message.\n";
    }
}

int main(int argc, char const *argv[])
{
    CLI::App app{"TW-Mailer Client"};
    string address{"127.0.0.1"};
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

    spdlog::debug("IP address: {}, Port: {}", address, to_string(port));

    socketFileDescriptor = connect(address.c_str(), port);

    int selection = 0;
    while (!abort)
    {
        // Input action
        cout << "Please select your action: 1 - SEND, 2 - READ, 3 - LIST, 4 - DEL, 5- QUIT\n";
        cout << ">>";
        selection = readNumericValue();

        // Respective action is executed
        switch (selection)
        {
        case 1:
            sendMessage(socketFileDescriptor);
            break;
        case 2:
            readMessage(socketFileDescriptor);
            break;
        case 3:
            listMessages(socketFileDescriptor);
            break;
        case 4:
            deleteMessage(socketFileDescriptor);
            break;
        case 5:
            cout << "Quitting application...\n";
            abort = true;
            break;
        default:
            cerr << "Unknown argument!\n";
            break;
        }
    }

    close(socketFileDescriptor);
    return EXIT_SUCCESS;
}
