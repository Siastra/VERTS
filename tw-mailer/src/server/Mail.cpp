#include "Mail.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>

Mails::Mails(const std::string& saveFile)
    : mSaveFile(saveFile)
{
}

void Mails::AddMail(const Mail& mail)
{
    mMails.emplace_back(mail);
}

bool Mails::RemoveMail(size_t index)
{
    if (index >= mMails.size())
        return false;
    mMails.erase(std::begin(mMails) + index);
    return true;
}

std::string Mails::GetMailByIndex(size_t index)
{
    if (index >= mMails.size())
        return std::string();

    auto mail = mMails.at(index);
    std::ostringstream oss;
    oss << mail.sender << '\n'
        << mail.receiver << '\n'
        << mail.subject << '\n'
        << mail.content << '\n';
    return oss.str();
}


bool Mails::Serialize()
{
    std::ofstream file(mSaveFile, std::ios::trunc);
    if (file.is_open())
    {
        for (const auto& mail : mMails)
        {
            file << mail.sender << '\n';
            file << mail.receiver << '\n';
            file << mail.subject << '\n';
            file << mail.content << '\n';
            file << '.' << '\n';
        }
        file.close();
        mMails.clear();
        return true;
    }
    return false;
}

bool Mails::Deserialize()
{
    constexpr int mailLines = 4;
    if (std::filesystem::exists(mSaveFile))
    {
        mMails.clear();
        std::ifstream file(mSaveFile);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                Mail mail;
                mail.sender = line;
                std::getline(file, line);
                mail.receiver = line;
                std::getline(file, line);
                mail.subject = line;
                while (std::getline(file, line))
                {
                    if (line == ".")
                    {
                        mail.content.erase(mail.content.find_last_of('\n'));
                        break;
                    }
                    mail.content.append(line + '\n');
                }
                AddMail(mail);
            }
            file.close();
            return true;
        }
        return false;
    }
    return true;
}
