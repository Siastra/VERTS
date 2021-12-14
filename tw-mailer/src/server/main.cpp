#include <iostream>
#include <filesystem>

#include "Server.hpp"

#include "libraries/CLI/CLI.hpp"

// create the directory to save the files in the directory you start the server from

int main(int argc, char** argv)
{
    CLI::App app{ "TW-Mailer Server" };
    uint port{ 6565 };
    std::string mailDirectory{ "mails" };

    app.add_option("-p,--port", port, "Port the server runs on")->required()->check(CLI::PositiveNumber);
    app.add_option("-d, --directory-path", mailDirectory, "Directory the mails are saved in")->required()->check(CLI::ExistingDirectory);

    CLI11_PARSE(app, argc, argv);

    Server server(port, mailDirectory);
    server.CreateSocket();
    server.Run();
    return EXIT_SUCCESS;
}