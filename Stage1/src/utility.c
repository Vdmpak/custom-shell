#include "customshell.h"
#include <dirent.h>
#include <errno.h>

extern char **environ;

// Prints all environment variables
void print_environment()
{
    for (char **env = environ; *env != NULL; env++)
    {
        printf("%s\n", *env);
    }
}

// Formats and prints arguments for the echo command
void normalize_echo_args(char **args)
{
    int first = 1;
    for (int i = 1; args[i] != NULL; i++)
    {
        if (!first)
            printf(" ");
        printf("%s", args[i]);
        first = 0;
    }
    putchar('\n');
}

// Validates if a given path is a directory
int validate_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return 0;
    }
    if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "%s: Not a directory\n", path);
        return 0;
    }
    return 1;
}
