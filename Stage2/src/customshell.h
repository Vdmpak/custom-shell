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
#include <fcntl.h>

#define MAX_BUFFER 1024
#define MAX_ARGS 64
#define SEPARATORS " \t\n"
#define MAX_BG_PROCESSES 10

typedef struct
{
    char *args[MAX_ARGS];
    char *input_file;
    char *output_file;
    int output_append;
    int error;
    int background;
} ParsedCommand;

extern pid_t bg_pids[MAX_BG_PROCESSES];
extern int bg_count;

void display_prompt(void);
int execute_internal_command(char **args);
void print_environment(void);
void normalize_echo_args(char **args);
int validate_directory(const char *path);
int is_internal(char **args);
ParsedCommand parse_command(char *buf);
void check_background_processes(void);

#endif
