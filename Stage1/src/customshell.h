#ifndef CUSTOMSHELL_H
#define CUSTOMSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>

#define MAX_BUFFER 1024
#define MAX_ARGS 64
#define SEPARATORS " \t\n"

// Function prototypes for shell operations
void display_prompt(void);
void execute_internal_command(char **args);
void print_environment(void);
void normalize_echo_args(char **args);
int validate_directory(const char *path);

#endif
