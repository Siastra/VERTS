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
#include <termios.h>
#include <unistd.h>

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
    vector<string> splitStringByNewLine(const char *text)
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

    int getch()
    {
        int ch;
        struct termios t_old, t_new;

        tcgetattr(STDIN_FILENO, &t_old);
        t_new = t_old;
        t_new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
        return ch;
    }

    string getpass(const char *prompt, bool show_asterisk = true)
    {
        const char BACKSPACE = 127;
        const char RETURN = 10;

        string password;
        unsigned char ch = 0;

        cout << prompt;

        while ((ch = getch()) != RETURN)
        {
            if (ch == BACKSPACE)
            {
                if (password.length() != 0)
                {
                    if (show_asterisk)
                        cout << "\b \b";
                    password.resize(password.length() - 1);
                }
            }
            else
            {
                password += ch;
                if (show_asterisk)
                    cout << '*';
            }
        }
        cout << endl;
        return password;
    }
}
