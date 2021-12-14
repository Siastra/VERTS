#include "Command.hpp"
#include "Utility.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>


extern std::string Command::mDirectoryPath;

bool Command::Send(std::string message)
{
    const std::string sender = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(sender))
        return false;
    const std::string receiver = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(receiver))
        return false;
    const std::string subject = SplitAtFirstOccurence(message, '\n');
    if (subject.length() > 80)
        return false;
    const std::string content = message.substr(0, message.length() - 2);
    const std::string receiverDirectory = mDirectoryPath + receiver + '/';

    if (!std::filesystem::exists(receiverDirectory))
        if (!std::filesystem::create_directory(receiverDirectory))
        {
            std::cout << "Couldn't create directory\n";
            return false;
        }

    const std::string filename = GenerateUniqueFilename(receiverDirectory + subject);
    // open file to write content to
    std::ofstream mail(filename);
    if (mail.is_open())
    {
        mail << sender << '\n';
        mail << receiver << '\n';
        mail << subject << '\n';
        mail << content;
    }
    else
    {
        std::cout << "Couldn't save mail\n";
        mail.close();
        return false;
    }
    mail.close();
    return true;
}

std::vector<std::string> Command::List(std::string message)
{
    const std::string username = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(username))
        return std::vector<std::string>();

    const std::string usernameDirectory = mDirectoryPath + username;
    if (!std::filesystem::exists(usernameDirectory))
        return std::vector<std::string>();

    std::vector<std::string> fileNames;
    for (const auto& dirEntry : std::filesystem::directory_iterator(usernameDirectory))
    {
        const std::string filename = std::filesystem::path(dirEntry).filename().string();
        fileNames.emplace_back(filename.substr(0, filename.size() - 6));
    }

    return fileNames;
}

std::string Command::Read(std::string message)
{
    const std::string username = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(username))
        return std::string();

    const std::string usernameDirectory = mDirectoryPath + username;
    if (!std::filesystem::exists(usernameDirectory))
        return std::string();

    int counter{ 0 };
    for (const auto& dirEntry : std::filesystem::directory_iterator(usernameDirectory))
    {
        if (counter == std::stoi(message))
        {
            std::ifstream mailStream;
            mailStream.open(std::filesystem::path(dirEntry));
            std::string content;
            if (mailStream.is_open())
            {
                std::string line;
                while (std::getline(mailStream, line))
                    content.append(line + '\n');
            }
            return content;
        }
        ++counter;
    }
    return std::string();
}

bool Command::Delete(std::string message)
{
    const std::string username = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(username))
        return false;

    const std::string usernameDirectory = mDirectoryPath + username;
    if (!std::filesystem::exists(usernameDirectory))
        return false;

    int counter{ 0 };
    for (auto& dirEntry : std::filesystem::directory_iterator(usernameDirectory))
    {
        if (counter == std::stoi(message))
            return std::filesystem::remove(dirEntry);
        counter++;
    }
    return false;
}

bool Command::Quit(const std::string& message)
{
    const std::string QUIT_MESSAGE = "QUIT\n";
    return message == QUIT_MESSAGE ? true : false;
}

void Command::SetMailDirectory(const std::string& directoryPath)
{
    mDirectoryPath = directoryPath;
    if (mDirectoryPath.back() != '/')
        mDirectoryPath.append("/");
}
