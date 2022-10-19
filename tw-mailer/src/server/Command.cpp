#include "Command.hpp"

#include "Utility.hpp"
#include "Mail.hpp"

#include <ldap.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <mutex>

std::string Command::mDirectoryPath;

Command::Command()
{
    mFailedAttempts = 0;
}

int Command::Login(std::string message)
{
    static std::string previousLoginName = "";
    std::string username = SplitAtFirstOccurence(message, '\n');
    if (CheckBlacklist(username))
        return -1;
    std::string password = SplitAtFirstOccurence(message, '\n');
    if (CheckLoginDataThreadWrapper(username, password))
    {
        mFailedAttempts = 0;
        mSessionData.first = username;
        return 1;
    }
    if (previousLoginName == username)
    {
        mFailedAttempts += 1;
        std::cout << "Failed attempts " << mFailedAttempts << std::endl;
        if (mFailedAttempts >= 3)
        {
            mFailedAttempts = 0;
            BlacklistUser(username, mSessionData.second, std::time(nullptr));
            return -1;
        }
    }
    else
    {
        mFailedAttempts = 1;
        std::cout << "Failed attempts " << mFailedAttempts << std::endl;
    }
    previousLoginName = username;
    return 0;
}

bool Command::Send(std::string message)
{
    // const std::string sender = SplitAtFirstOccurence(message, '\n');
    // if (!ValidateUsername(sender))
    //     return false;

    if (mSessionData.first.empty())
        return false;
    const std::string receiver = SplitAtFirstOccurence(message, '\n');
    if (!ValidateUsername(receiver))
        return false;
    const std::string subject = SplitAtFirstOccurence(message, '\n');
    if (subject.length() > 80)
        return false;
    const std::string content = message.substr(0, message.length() - 3);

    Mail mail(mSessionData.first, receiver, subject, content);
    Mails mails(mDirectoryPath + receiver);
    bool rv{ false };
    std::mutex mutex;
    mutex.lock();
    if (mails.Deserialize())
    {
        mails.AddMail(mail);
        rv = mails.Serialize();
    }
    mutex.unlock();
    return rv;
}

std::vector<std::string> Command::List(std::string message)
{
    // const std::string username = SplitAtFirstOccurence(message, '\n');
    // if (!ValidateUsername(username))
    //     return std::vector<std::string>();

    if (mSessionData.first.empty())
        return std::vector<std::string>();
    const std::string usernameFile = mDirectoryPath + mSessionData.first;
    if (!std::filesystem::exists(usernameFile))
        return std::vector<std::string>();

    std::vector<std::string> subjects;
    Mails mails(usernameFile);
    std::mutex mutex;
    mutex.lock();
    if (mails.Deserialize())
    {
        auto userMails = mails.GetMails();
        for (const auto& mail : userMails)
            subjects.emplace_back(mail.subject);
    }
    mutex.unlock();
    return subjects;
}

std::string Command::Read(std::string message)
{
    // const std::string username = SplitAtFirstOccurence(message, '\n');
    // if (!ValidateUsername(username))
    //     return std::string();

    if (mSessionData.first.empty())
        return std::string();
    const std::string usernameFile = mDirectoryPath + mSessionData.first;
    if (!std::filesystem::exists(usernameFile))
        return std::string();

    std::string rv{};
    Mails mails(usernameFile);
    std::mutex mutex;
    mutex.lock();
    if (mails.Deserialize())
    {
        size_t index = std::stoi(message);
        rv = mails.GetMailByIndex(index);
    }
    mutex.unlock();
    return rv;
}

bool Command::Delete(std::string message)
{
    // const std::string username = SplitAtFirstOccurence(message, '\n');
    // if (!ValidateUsername(username))
    //     return false;

    if (mSessionData.first.empty())
        return false;
    const std::string usernameFile = mDirectoryPath + mSessionData.first;
    if (!std::filesystem::exists(usernameFile))
        return false;

    Mails mails(usernameFile);
    bool rv{ false };
    std::mutex mutex;
    mutex.lock();
    if (mails.Deserialize())
    {
        rv = mails.RemoveMail(std::stoi(message));
        rv = rv && mails.Serialize();
    }
    mutex.unlock();
    return rv;
}

bool Command::Quit(const std::string& message)
{
    const std::string QUIT_MESSAGE = "QUIT\n";
    if (message == QUIT_MESSAGE)
    {
        mSessionData = {};
        return true;
    }
    return false;
}

void Command::SetMailDirectory(const std::string& directoryPath)
{
    mDirectoryPath = directoryPath;
    if (mDirectoryPath.back() != '/')
        mDirectoryPath.append("/");
    if (!std::filesystem::exists(mDirectoryPath))
        std::filesystem::create_directories(mDirectoryPath);
}

void Command::SetSessionAddress(const std::string& address)
{
    mSessionData.second = address;
}

bool Command::CheckLoginData(std::string& username, std::string& password)
{
    LDAP* ldapHandle;
    const int ldapVersion = LDAP_VERSION3;

    int status = ldap_initialize(&ldapHandle, "ldap://ldap.technikum-wien.at:389");
    if (status != LDAP_SUCCESS)
    {
        std::cerr << "LDAP init error: " << ldap_err2string(status) << std::endl;
        return false;
    }

    status = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);
    if (status != LDAP_OPT_SUCCESS)
    {
        std::cerr << "LDAP set option error: " << ldap_err2string(status) << std::endl;
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return false;
    }

    status = ldap_start_tls_s(ldapHandle, NULL, NULL);
    if (status != LDAP_SUCCESS)
    {
        std::cerr << "LDAP start tls error: " << ldap_err2string(status) << std::endl;
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return false;
    }

    BerValue bindCredentials;
    bindCredentials.bv_val = password.data();
    bindCredentials.bv_len = password.length();
    BerValue* servercredp;
    char ldapBindUser[256];

    sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", username.data());
    std::cout << ldapBindUser << std::endl;
    status = ldap_sasl_bind_s(
        ldapHandle,
        ldapBindUser,
        LDAP_SASL_SIMPLE,
        &bindCredentials,
        NULL,
        NULL,
        &servercredp);
    if (status != LDAP_SUCCESS)
    {
        std::cerr << "LDAP bind error: " << ldap_err2string(status) << std::endl;
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return false;
    }

    LDAPMessage* searchResult;
    const std::string ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
    const std::string ldapSearchFilter = "(uid=" + username + ")";
    ber_int_t ldapSearchScope = LDAP_SCOPE_SUBTREE;
    const char* ldapSearchResultAttributes[] = { "uid", "cn", NULL };

    status = ldap_search_ext_s(
        ldapHandle,
        ldapSearchBaseDomainComponent.data(),
        ldapSearchScope,
        ldapSearchFilter.data(),
        (char**)ldapSearchResultAttributes,
        0,
        NULL,
        NULL,
        NULL,
        500,
        &searchResult);
    if (status != LDAP_SUCCESS)
    {
        std::cerr << "LDAP search error: " << ldap_err2string(status) << std::endl;
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return false;
    }

    auto value = ldap_count_entries(ldapHandle, searchResult);
    std::cout << value << std::endl;
    return value > 0 ? true : false;
}

bool Command::CheckBlacklist(const std::string& username)
{
    const std::string blacklistFileName = "BLACKLIST";
    std::vector<std::tuple<std::string, std::string, time_t>> blacklistedUser;
    std::ifstream file(mDirectoryPath + blacklistFileName);

    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            const std::string bannedUsername = SplitAtFirstOccurence(line, ' ');
            const std::string bannedAddress = SplitAtFirstOccurence(line, ' ');
            const std::string banTime = line;

            std::tm tm = {};
            std::stringstream ss(banTime);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            auto tp = std::mktime(&tm);
            auto currentTime = std::time(nullptr);

            if (currentTime - tp >= 60)
                continue;
            blacklistedUser.emplace_back(std::tuple<std::string, std::string, time_t>{ bannedUsername, bannedAddress, tp });
        }
        file.close();
    }
    else
        return false;
    ClearBlacklist();
    for (const auto& user : blacklistedUser)
        BlacklistUser(std::get<0>(user), std::get<1>(user), std::get<2>(user));
    if (ContainsUser(blacklistedUser, username, mSessionData.second))
        return true;
    return false;
}

void Command::BlacklistUser(const std::string& username, const std::string& address, const time_t& banTime)
{
    const std::string blacklistFileName = "BLACKLIST";
    std::ofstream file(mDirectoryPath + blacklistFileName, std::ios::app);
    if (file.is_open())
    {
        std::ostringstream oss;
        oss << username << ' ' << address << ' ' << SerializeTimePoint(banTime, "%Y-%m-%d %H:%M:%S") << '\n';
        file << oss.str();
    }
    file.close();
}

void Command::ClearBlacklist()
{
    const std::string blacklistFileName = "BLACKLIST";
    std::filesystem::remove(mDirectoryPath + blacklistFileName);
}

bool Command::ContainsUser(const std::vector<std::tuple<std::string, std::string, time_t>>& container, const std::string& username, const std::string& ipAddress)
{
    for (const auto& element : container)
        if (std::get<0>(element) == username && std::get<1>(element) == ipAddress)
            return true;
    return false;
}

bool Command::CheckLoginDataThreadWrapper(std::string& username, std::string& password)
{
    std::mutex mutex;
    mutex.lock();
    auto rv = CheckLoginData(username, password);
    mutex.unlock();
    return rv;
}