#pragma once
#include <string>
#include <vector>

struct Mail
{
    std::string sender;
    std::string receiver;
    std::string subject;
    std::string content;

    Mail()
        : sender(""), receiver(""), subject(""), content("")
    {
    }

    Mail(const std::string& sender, const std::string& receiver, const std::string& subject, const std::string& content)
        : sender(sender), receiver(receiver), subject(subject), content(content)
    {
    }
};

class Mails
{
public:
    Mails(const std::string& saveFile);
    void AddMail(const Mail& mail);
    bool RemoveMail(size_t index);
    std::vector<Mail> GetMails() const { return mMails; };
    std::string GetMailByIndex(size_t index);
    bool Serialize();
    bool Deserialize();

private:
    std::vector<Mail> mMails;
    std::string mSaveFile;

};