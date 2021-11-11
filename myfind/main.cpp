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
#include <sys/stat.h>
#include <cstring>

/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage() {
    fprintf(stderr, "Usage: %s [-R] [-i] searchPath filename1 [filename2] ... [filenameN]\n", program_name);
    exit(EXIT_FAILURE);
}

char *getCWD() {
    long maxpath;
    char *mycwdp;

    if ((maxpath = pathconf(".", _PC_PATH_MAX)) == -1) {
        perror("Failed to determine the pathname length");
    }
    if ((mycwdp = (char *) malloc(maxpath)) == nullptr) {
        perror("Failed to allocate space for pathname");
    }
    if (getcwd(mycwdp, maxpath) == nullptr) {
        perror("Failed to get current working directory");
    }
    return mycwdp;
}

int is_regular_file(const char *path)
{
    struct stat path_stat{};
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

char* searchForFilename(DIR *dir, char* filename)
{
    struct dirent *elem;   // pointer represent directory stream
    while ((elem = readdir(dir)) != nullptr) {
        char filepath[PATH_MAX + 1];
        realpath(elem->d_name, filepath);
        if (is_regular_file(filepath) && (strcmp(elem->d_name, filename) == 0))
        {
            printf("File found: %s\n", elem->d_name);
            return filepath;
        }
        else if (is_regular_file(filepath))
        {
            printf("File: %s\n", elem->d_name);
        }
        else
        {
            printf("Dir: %s\n", elem->d_name);
        }
    }
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[]) {

    /*
     * DEFINITIONEN
     */

    int c;
    int error = 0;
    int caseSensitive = 0;
    int recursive = 0;
    char *searchPath = nullptr;
    char *mycwd = nullptr;
    std::vector<char *> filenames = {};
    program_name = argv[0];


    /*
     * ARGS
     */
    while ((c = getopt(argc, argv, "iR")) != EOF) {
        switch (c) {
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
    while (optind < argc) {
        if (searchPath == nullptr) {
            searchPath = argv[optind];
        } else {
            filenames.push_back(argv[optind]);
        }
        optind++;
    }

    //Change to directory
    mycwd = getCWD();
    DIR *dir;
    if ((dir = opendir(searchPath)) != nullptr) {      // check if directory  open
        chdir(searchPath);

    }
    closedir(dir); //close directory.
    return EXIT_SUCCESS;
}
