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
#include <dirent.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <cstring>

namespace fs = std::filesystem;

/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    fprintf(stderr, "Usage: %s [-R] [-i] searchPath filename1 [filename2] ... [filenameN]\n", program_name);
    exit(EXIT_FAILURE);
}

bool checkFilename(fs::directory_entry dir_entry, char *filename)
{
    std::string path = dir_entry.path();
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
    if (dir_entry.is_directory())
    {
        //printf("Dir: %s\n", base_filename.c_str());
    }
    else
    {
        if (strcmp(base_filename.c_str(), filename) == 0)
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
            if (checkFilename(dir_entry, filename))
            {
                foundPath = dir_entry.path();
            }
        }
    }
    else
    {
        for (auto const &dir_entry : std::filesystem::directory_iterator{searchPath})
        {
            if (checkFilename(dir_entry, filename))
            {
                foundPath = dir_entry.path();
            }
        }
    }
    if (foundPath.compare(""))
        printf("Found path: %s\n", foundPath.c_str());  
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[])
{

    /*
     * DEFINITIONEN
     */

    int c;
    int error = 0;
    int caseSensitive = 0;
    int recursive = 0;
    char *searchPath = nullptr;
    std::vector<char *> filenames = {};
    program_name = argv[0];

    /*
     * ARGS
     */
    while ((c = getopt(argc, argv, "iR")) != EOF)
    {
        switch (c)
        {
        case 'R':
            if (recursive) /* mehrmalige Verwendung? */
            {
                error = 1;
                break;
            }
            recursive = 1;
            break;
        case 'i':
            if (caseSensitive) /* mehrmalige Verwendung? */
            {
                error = 1;
                break;
            }
            caseSensitive = 1;
            break;
        case '?': /* ungueltiges Argument */
            error = 1;
            break;
        default: /* unmoegliech */
            assert(0);
        }
    }

    if (error) /* Optionen fehlerhaft ? */
    {
        print_usage();
    }

    /* Die restlichen Argumente, die keine Optionen sind, befinden sich in
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

    searchForFilename(searchPath, filenames.at(0), recursive, caseSensitive);

    return EXIT_SUCCESS;
}
