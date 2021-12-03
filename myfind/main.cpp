/*
 * author: Marcel Glavanits
 * class: BIF DUAL 3
 * file: main.cpp
 * date: 09.11.21
 */

#include <cstdio>
#include <getopt.h>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>

#define KEY 52004322 /* eindeutiger Key z.B. Matrikelnummer */
#define PERM 0660

#define MAX_DATA 260

typedef struct
{
    long mType;               //Messagepriority
    pid_t mId;                //Process-ID
    char mFilename[MAX_DATA]; //Filename to search
    char mPath[MAX_DATA];     //Filepath to searched file
} message_t;

namespace fs = std::filesystem;

/* global variable for programname */
char *program_name = nullptr;

/* Function print_usage() for displaying the usage */
void print_usage()
{
    fprintf(stderr, "Usage: %s [-R] [-i] searchPath filename1 [filename2] ... [filenameN]\n", program_name);
    exit(EXIT_FAILURE);
}

/*
 * Function checkFilename for checking, if a given dir entry is the searched file
 * @param dirEntry: directory entry, which should be checked
 * @param filename: name of the file searched for
 * @param caseSensitive: whether the namecheck shoould be casesensitive or not
 * 
 * @return whether the directory entry is the one searched for or not 
 */
bool checkFilename(fs::directory_entry dirEntry, char *filename, bool caseSensitive)
{
    std::string path = dirEntry.path();
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
    if ((!dirEntry.is_directory()) && ((!caseSensitive && (strcmp(base_filename.c_str(), filename) == 0)) || (caseSensitive && (strcasecmp(base_filename.c_str(), filename) == 0))))
        return true;
    return false;
}

/*
 * Function searchForFilename for searching a given directory for given file
 * @param searchPath: directory, which should be checked
 * @param filename: name of the file searched for
 * @param recursive: whether the search shoould be recursive or not
 * @param caseSensitive: whether the namecheck shoould be case sensitive or not
 * 
 */
void searchForFilename(char *searchPath, char *filename, bool recursive, bool caseSensitive, char *returnPath)
{
    std::string foundPath = "not found";

    try
    {
        /* code */

        if (recursive)
        {
            for (auto &dirEntry : fs::recursive_directory_iterator(searchPath))
            {
                if (checkFilename(dirEntry, filename, caseSensitive)) //found
                    foundPath = fs::canonical(dirEntry.path());
            }
        }
        else
        {
            for (auto const &dirEntry : fs::directory_iterator{searchPath})
            {
                if (checkFilename(dirEntry, filename, caseSensitive)) //found
                    foundPath = fs::canonical(dirEntry.path());
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        fprintf(stderr, "SearchPath: No such directory!\n");
    }

    for (size_t i = 0; i < foundPath.length(); i++)
    {
        returnPath[i] = foundPath[i];
    }
    returnPath[foundPath.length()] = '\0';
}

/* main Funktion with argument handling */
int main(int argc, char *argv[])
{

    /*
     * DEFINITIONS
     */

    int c;                                //Argument
    pid_t pid;                            //Process-ID
    int msg_id = -1;                      //ID of Message Queue
    message_t msg;                        //Messagebuffer
    char filepath[MAX_DATA];              //Path to file
    bool error = 0;                        //Is used for error handling during argument handling
    bool caseSensitive = 0;                //Commandline-Argument case sensitive
    bool recursive = 0;                    //Commandline-Argument recursive
    char *searchPath = nullptr;           //Commandline-Argument searchpath
    std::vector<char *> filenames = {};   //Commandline-Arguments filenames to search
    std::vector<message_t> messages = {}; //Messages form child processes
    program_name = argv[0];               //Programname = myfind

    /*
     * ARGS
     */
    while ((c = getopt(argc, argv, "iR")) != EOF)
    {
        switch (c)
        {
        case 'R':
            if (recursive)
            {
                error = 1;
                break;
            }
            recursive = 1;
            break;
        case 'i':
            if (caseSensitive)
            {
                error = 1;
                break;
            }
            caseSensitive = 1;
            break;
        case '?': /* invalid argument */
            error = 1;
            break;
        default: /* impossible to reach */
            assert(0);
        }
    }

    if (error) // options invalid
    {
        print_usage();
    }

    /* 
     * remaining arguments are in
     * argv[optind] bis argv[argc-1]
     */
    while (optind < argc)
    {
        if (searchPath == nullptr)
        {
            searchPath = argv[optind];
        }
        else
        {
            filenames.push_back(argv[optind]);
        }
        optind++;
    }

    if (!fs::exists(searchPath))
    {
        fprintf(stderr, "%s: %s does not exist!\n", argv[0], searchPath);
        return EXIT_FAILURE;
    }

    if ((msg_id = msgget(KEY, PERM | IPC_CREAT)) == -1) //create message queue and return id
    {
        // error handling 
        fprintf(stderr, "%s: Can't access message queue\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < filenames.size(); i++)
    {
        pid = fork();
        if (pid == pid_t(0))
        {
            // Search for filename
            searchForFilename(searchPath, filenames.at(i), recursive, caseSensitive, filepath);

            //Send message
            msg.mType = 1;
            msg.mId = getpid();
            strncpy(msg.mFilename, filenames.at(i), MAX_DATA);
            strncpy(msg.mPath, filepath, MAX_DATA);
            if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(long), 0) == -1)
            {
                // error handling
                fprintf(stderr, "%s: Can't send message\n", program_name);
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        else if (pid > pid_t(0))
        {
            // Parent
        }
        else
        {
            // Error: fork failed
            fprintf(stderr, "%s: fork failed!\n", program_name);
        }
    }

    while (messages.size() < filenames.size()) //wait for every child process to send a message
    {
        if (msgrcv(msg_id, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1)
        {
            // error handling
            fprintf(stderr, "%s: Can't receive from message queue\n", program_name);
            return EXIT_FAILURE;
        }
        messages.push_back(msg);
    }

    msgctl(msg_id, IPC_RMID, NULL); //destroy the message queue

    for (auto message : messages) //Print all messages
    {
        printf("%ld: %s: %s\n", (long)message.mId, message.mFilename, message.mPath);
    }

    return EXIT_SUCCESS;
}
