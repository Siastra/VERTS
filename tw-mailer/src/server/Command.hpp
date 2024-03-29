#pragma once
#include <string>
#include <vector>

class Command
{
public:
    Command();
    int Login(std::string message);
    bool Send(std::string message);
    std::vector<std::string> List(std::string message);
    std::string Read(std::string message);
    bool Delete(std::string message);
    bool Quit(const std::string& message);

    void SetSessionAddress(const std::string& address);
    static void SetMailDirectory(const std::string& path);

private:
    bool CheckLoginData(std::string& username, std::string& password);
    bool CheckLoginDataThreadWrapper(std::string& username, std::string& password);
    bool CheckBlacklist(const std::string& username);
    void BlacklistUser(const std::string& username, const std::string& address, const time_t& banTime);
    void ClearBlacklist();
    bool ContainsUser(const std::vector<std::tuple<std::string, std::string, time_t>>& container, const std::string& username, const std::string& ipAddress);

private:
    static std::string mDirectoryPath;
    std::pair<std::string, std::string> mSessionData;
    int mFailedAttempts;
};