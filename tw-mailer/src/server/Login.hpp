#pragma once
#include <string>
#include <vector>

class Login
{
public:
    bool CheckLoginData(std::string& username, std::string& password);
    bool CheckLoginDataThreadWrapper(std::string& username, std::string& password);
    bool CheckBlacklist(const std::string& username);
    void BlacklistUser(const std::string& username, const std::string& address, const time_t& banTime);
    void ClearBlacklist();
    bool ContainsUser(const std::vector<std::tuple<std::string, std::string, time_t>>& container, const std::string& username, const std::string& ipAddress);
};