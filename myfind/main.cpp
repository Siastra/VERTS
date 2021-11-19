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
 * @param dir_entry: directory entry, which should be checked
 * @param filename: name of the file searched for
 * @param caseSensitive: whether the namecheck shoould be casesensitive or not
 * 
 * @return whether the directory entry is the one searched for or not 
 */
bool checkFilename(fs::directory_entry dir_entry, char *filename, int caseSensitive)
{
    std::string path = dir_entry.path();
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
    if (dir_entry.is_directory())
    {
        //printf("Dir: %s\n", base_filename.c_str());
    }
    else
    {
        if ((!caseSensitive && (strcasecmp(base_filename.c_str(), filename) == 0)) || (caseSensitive && (strcmp(base_filename.c_str(), filename) == 0)))
        {
            //printf("File found: %s\n", base_filename.c_str());
            return true;
        }
        else
        {
            //printf("File: %s\n", base_filename.c_str());
        }
    }
    return false;
}

void searchForFilename(char *searchPath, char *filename, int recursive, int caseSensitive)
{
    std::string foundPath = "";
    if (recursive)
    {
        for (auto &dir_entry : fs::recursive_directory_iterator(searchPath))
        {
            if (checkFilename(dir_entry, filename, caseSensitive))
            {
                foundPath = dir_entry.path();
            }
        }
    }
    else
    {
        for (auto const &dir_entry : std::filesystem::directory_iterator{searchPath})
        {
            if (checkFilename(dir_entry, filename, caseSensitive))
            {
                foundPath = dir_entry.path();
            }
        }
    }
    if (foundPath.compare(""))
        printf("Found path: %s\n", foundPath.c_str());
}

/* main Funktion with argument handling */
int main(int argc, char *argv[])
{

    /*
     * DEFINITIONS
     */

    int c;                                //Argument
    pid_t pid, wpid;                      //Process-ID
    int status = 0;                       //Processstatus
    int msg_id = -1;                      //ID of Message Queue
    message_t msg;                        //Messagebuffer
    int error = 0;                        //Is used for error handling during argument handling
    int caseSensitive = 0;                //Commandline-Argument case sensitive
    int recursive = 0;                    //Commandline-Argument recursive
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

    if (error) /* options invalid ? */
    {
        print_usage();
    }

    /* remaining arguments are in
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

    if ((msg_id = msgget(KEY, PERM | IPC_CREAT)) == -1) //create message queue and return id
    {
        /* error handling */
        fprintf(stderr, "%s: Can't access message queue\n", argv[0]);
        return EXIT_FAILURE;
    }
    printf("Message Queue created!\n");

    for (size_t i = 0; i < filenames.size(); i++)
    {
        pid = fork();
        if (pid == pid_t(0))
        {
            // Child process
            sleep(5);
            //Send message
            msg.mType = 1;
            msg.mId = getpid();
            strncpy(msg.mFilename, filenames.at(i), MAX_DATA);
            strncpy(msg.mPath, "Test", MAX_DATA);
            if (msgsnd(msg_id, &msg, sizeof(msg) - sizeof(long), 0) == -1)
            {
                /* error handling */
                fprintf(stderr, "%s: Can't send message\n", argv[0]);
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

    while ((wpid = wait(&status)) > 0)
        ; // parent waits for all the child processes to end

    while (1)
    {
        if (msgrcv(msg_id, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1)
        {
            /* error handling */
            fprintf(stderr, "%s: Can't receive from message queue\n", argv[0]);
            return EXIT_FAILURE;
        }
        printf("%ld: %s: %s\n", (long)msg.mId, msg.mFilename, msg.mPath);
    }

    msgctl(msg_id, IPC_RMID, NULL); //destroy the message queue
    printf("Message Queue deleted!\n");

    //searchForFilename(searchPath, filenames.at(0), recursive, caseSensitive);

    return EXIT_SUCCESS;
}
