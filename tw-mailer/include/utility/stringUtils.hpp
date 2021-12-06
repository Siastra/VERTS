#include <string>
#include <regex>
#include <vector>
#include <iostream>

using namespace std;

namespace stringUtility
{

    bool checkUsername(string username)
    {
        regex regex{"([a-z]|[0-9]){1,8}"};
        if (regex_match(username, regex))
            return true;
        return false;
    }

    vector<string> splitStringByNewLine(const char* text)
    {
        vector<string> ret{};
        stringstream ss(text);
        string to;
        if (text != NULL)
        {
            while (getline(ss, to, '\n'))
            {
                ret.push_back(to);
            }
        }

        return ret;
    }
}