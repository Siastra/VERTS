#include <string>
#include <regex>

namespace stringUtility
{

    bool checkUsername(std::string username)
    {
        std::regex regex{"([a-z]|[0-9]){1,8}"};
        if (std::regex_match(username, regex))
            return true;
        return false;
    }

}