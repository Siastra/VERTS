#pragma once
#include <string>
#include <vector>

class Command
{
public:
    static bool Send(std::string message);
    static std::vector<std::string> List(std::string message);
    static std::string Read(std::string message);
    static bool Delete(std::string message);
    static bool Quit(const std::string& message);

    static void SetMailDirectory(const std::string& path);

private:
    static std::string mDirectoryPath;
};