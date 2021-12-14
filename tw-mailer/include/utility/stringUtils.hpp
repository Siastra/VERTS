/*
 * authors: Benedikt Schottleitner & Marcel Glavanits
 * class: BIF DUAL 3
 * file: stringUtils.hpp
 * date: 13.12.21
 */

#include <string>
#include <regex>
#include <vector>
#include <iostream>

using namespace std;

namespace stringUtility
{

    /*
     * Function checkUsername for checking, if a given username only contains lower case letters
     * and is not longer than 8 characters.
     * @param std::string username: string which should be validated
     * @return whether the given username is valid
     */
    bool checkUsername(string username)
    {
        regex regex{"([a-z]|[0-9]){1,8}"};
        if (regex_match(username, regex))
            return true;
        return false;
    }

    /*
     * Function splitStringByNewLine for splitting a given string by new line characters.
     * @param const char* text: string which should be splitted
     * @return a vector with single lines from text 
     */
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