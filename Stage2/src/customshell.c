#include "customshell.h"

pid_t bg_pids[MAX_BG_PROCESSES] = {0};
int bg_count = 0;

// Displays shell prompt with current working directory
void display_prompt()
{
    char cwd[MAX_BUFFER];
    if (getcwd(cwd, sizeof(cwd)))
    {
        printf("%s> ", cwd);
    }
    else
    {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
}

// Identifies if the command is a built-in shell command
int is_internal(char **args)
{
    if (!args[0])
        return 0;
    const char *internals[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", NULL};
    for (int i = 0; internals[i]; i++)
    {
        if (strcmp(args[0], internals[i]) == 0)
            return 1;
    }
    return 0;
}

// Parses input string into command, arguments, and redirection components
ParsedCommand parse_command(char *buf)
{
    ParsedCommand pc = {.args = {NULL}, .input_file = NULL, .output_file = NULL, .output_append = 0, .error = 0, .background = 0};
    char *tokens[MAX_ARGS];
    int token_count = 0;

    char *token = strtok(buf, SEPARATORS);
    while (token && token_count < MAX_ARGS - 1)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, SEPARATORS);
    }
    tokens[token_count] = NULL;

    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0)
    {
        pc.background = 1;
        tokens[token_count - 1] = NULL;
        token_count--;
    }

    int arg_count = 0;
    for (int i = 0; i < token_count; i++)
    {
        if (strcmp(tokens[i], "&") == 0)
        {
            fprintf(stderr, "Error: Misplaced '&' operator\n");
            pc.error = 1;
            return pc;
        }
        if (strcmp(tokens[i], "<") == 0)
        {
            if (pc.input_file)
            {
                fprintf(stderr, "Error: Multiple input redirections specified\n");
                pc.error = 1;
                return pc;
            }
            if (i + 1 >= token_count || !tokens[i + 1])
            {
                fprintf(stderr, "Error: No input file specified for <\n");
                pc.error = 1;
                return pc;
            }
            pc.input_file = tokens[++i];
        }
        else if (strcmp(tokens[i], ">") == 0)
        {
            if (pc.output_file)
            {
                fprintf(stderr, "Error: Multiple output redirections specified\n");
                pc.error = 1;
                return pc;
            }
            if (i + 1 >= token_count || !tokens[i + 1])
            {
                fprintf(stderr, "Error: No output file specified for >\n");
                pc.error = 1;
                return pc;
            }
            pc.output_file = tokens[++i];
            pc.output_append = 0;
        }
        else if (strcmp(tokens[i], ">>") == 0)
        {
            if (pc.output_file)
            {
                fprintf(stderr, "Error: Multiple output redirections specified\n");
                pc.error = 1;
                return pc;
            }
            if (i + 1 >= token_count || !tokens[i + 1])
            {
                fprintf(stderr, "Error: No output file specified for >>\n");
                pc.error = 1;
                return pc;
            }
            pc.output_file = tokens[++i];
            pc.output_append = 1;
        }
        else
        {
            pc.args[arg_count++] = tokens[i];
        }
    }
    pc.args[arg_count] = NULL;

    if (arg_count == 0 && pc.background)
    {
        fprintf(stderr, "Error: Invalid command\n");
        pc.error = 1;
    }

    return pc;
}

// Executes built-in commands with appropriate handling
int execute_internal_command(char **args)
{
    if (!args[0])
        return 0;

    if (strcmp(args[0], "cd") == 0)
    {
        if (!args[1])
        {
            char cwd[MAX_BUFFER];
            if (getcwd(cwd, sizeof(cwd)))
            {
                printf("%s\n", cwd);
            }
            else
            {
                perror("cd");
            }
            return 1;
        }

        if (!validate_directory(args[1]))
            return 1;

        if (chdir(args[1]) != 0)
        {
            fprintf(stderr, "cd: %s: %s\n", args[1], strerror(errno));
        }
        else
        {
            char cwd[MAX_BUFFER];
            getcwd(cwd, sizeof(cwd));
            setenv("PWD", cwd, 1);
        }
        return 1;
    }
    else if (strcmp(args[0], "clr") == 0)
    {
        system("clear");
        return 1;
    }
    else if (strcmp(args[0], "dir") == 0)
    {
        char cmd[MAX_BUFFER] = "ls -al";
        if (args[1])
        {
            if (!validate_directory(args[1]))
                return 1;
            strncat(cmd, " ", MAX_BUFFER - strlen(cmd) - 1);
            strncat(cmd, args[1], MAX_BUFFER - strlen(cmd) - 1);
        }
        system(cmd);
        return 1;
    }
    else if (strcmp(args[0], "environ") == 0)
    {
        print_environment();
        return 1;
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        normalize_echo_args(args);
        return 1;
    }
    else if (strcmp(args[0], "help") == 0)
    {
        char *root = getenv("CUSTOMSHELL_ROOT");
        if (!root)
        {
            fprintf(stderr, "help: CUSTOMSHELL_ROOT not set\n");
            return 1;
        }

        char path[MAX_BUFFER];
        size_t path_len = snprintf(path, sizeof(path), "%s/manual/help.txt", root);
        if (path_len >= sizeof(path))
        {
            fprintf(stderr, "help: Path is too long!\n");
            return 1;
        }

        if (access(path, F_OK) == -1)
        {
            fprintf(stderr, "help: %s: %s\n", path, strerror(errno));
            return 1;
        }

        char cmd[MAX_BUFFER + 10];
        size_t cmd_len = snprintf(cmd, sizeof(cmd), "more \"%s\"", path);
        if (cmd_len >= sizeof(cmd))
        {
            fprintf(stderr, "help: Command too long\n");
            return 1;
        }
        system(cmd);
        return 1;
    }
    else if (strcmp(args[0], "pause") == 0)
    {
        printf("Press Enter to continue...");
        fflush(stdout);
        FILE *tty = fopen("/dev/tty", "r");
        if (tty)
        {
            while (fgetc(tty) != '\n')
                ;
            fclose(tty);
        }
        return 1;
    }
    else if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
        return 1;
    }

    return 0;
}

// Monitors and cleans up completed background processes
void check_background_processes(void)
{
    for (int i = 0; i < bg_count; i++)
    {
        pid_t pid = bg_pids[i];
        int status;
        if (waitpid(pid, &status, WNOHANG) > 0)
        {
            printf("[%d] Done %d\n", i + 1, pid);
            for (int j = i; j < bg_count - 1; j++)
            {
                bg_pids[j] = bg_pids[j + 1];
            }
            bg_pids[--bg_count] = 0;
            i--;
        }
    }
}

// Shell entry point and main command loop
int main(int argc, char *argv[])
{
    char shell_path[MAX_BUFFER], root_path[MAX_BUFFER];

    if (!realpath(argv[0], shell_path))
    {
        perror("realpath() failed for shell executable");
        exit(EXIT_FAILURE);
    }

    strncpy(root_path, shell_path, sizeof(root_path));
    dirname(root_path);
    dirname(root_path);

    setenv("shell", shell_path, 1);
    setenv("CUSTOMSHELL_ROOT", root_path, 1);

    FILE *input = stdin;
    if (argc == 2)
    {
        input = fopen(argv[1], "r");
        if (!input)
        {
            perror("fopen() failed");
            exit(EXIT_FAILURE);
        }
    }

    char buf[MAX_BUFFER];
    while (1)
    {
        if (input == stdin)
            display_prompt();
        if (!fgets(buf, MAX_BUFFER, input))
            break;

        ParsedCommand pc = parse_command(buf);
        if (pc.error)
            continue;
        if (!pc.args[0])
        {
            if (pc.input_file || pc.output_file)
            {
                fprintf(stderr, "Error: No command provided with redirection\n");
            }
            continue;
        }

        check_background_processes();

        if (is_internal(pc.args))
        {
            if (pc.background)
            {
                fprintf(stderr, "Error: Internal commands cannot be run in the background\n");
                continue;
            }
            if (pc.input_file)
            {
                continue;
            }
            if (pc.output_file)
            {
                int stdout_save = dup(1);
                if (stdout_save == -1)
                {
                    perror("dup() failed for stdout");
                    continue;
                }
                int out_fd = open(pc.output_file, O_WRONLY | O_CREAT | (pc.output_append ? O_APPEND : O_TRUNC), 0644);
                if (out_fd == -1)
                {
                    fprintf(stderr, "Error: Cannot open output file '%s': %s\n", pc.output_file, strerror(errno));
                    close(stdout_save);
                    continue;
                }
                if (dup2(out_fd, 1) == -1)
                {
                    perror("dup2() failed for stdout");
                    close(out_fd);
                    close(stdout_save);
                    continue;
                }
                close(out_fd);
                execute_internal_command(pc.args);
                if (dup2(stdout_save, 1) == -1)
                {
                    perror("dup2() failed to restore stdout");
                }
                close(stdout_save);
            }
            else
            {
                execute_internal_command(pc.args);
            }
        }
        else
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork() failed");
            }
            else if (pid == 0)
            {
                if (pc.input_file)
                {
                    int in_fd = open(pc.input_file, O_RDONLY);
                    if (in_fd == -1)
                    {
                        fprintf(stderr, "Error: Cannot open input file '%s': %s\n", pc.input_file, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(in_fd, 0) == -1)
                    {
                        perror("dup2() failed for stdin");
                        close(in_fd);
                        exit(EXIT_FAILURE);
                    }
                    close(in_fd);
                }
                if (pc.output_file)
                {
                    int out_fd = open(pc.output_file, O_WRONLY | O_CREAT | (pc.output_append ? O_APPEND : O_TRUNC), 0644);
                    if (out_fd == -1)
                    {
                        fprintf(stderr, "Error: Cannot open output file '%s': %s\n", pc.output_file, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(out_fd, 1) == -1)
                    {
                        perror("dup2() failed for stdout");
                        close(out_fd);
                        exit(EXIT_FAILURE);
                    }
                    close(out_fd);
                }
                if (setenv("parent", shell_path, 1) == -1)
                {
                    perror("setenv() failed");
                    exit(EXIT_FAILURE);
                }
                if (execvp(pc.args[0], pc.args) == -1)
                {
                    fprintf(stderr, "%s: %s\n", pc.args[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (pc.background)
                {
                    if (bg_count < MAX_BG_PROCESSES)
                    {
                        bg_pids[bg_count++] = pid;
                        printf("[%d] %d\n", bg_count, pid);
                    }
                    else
                    {
                        fprintf(stderr, "Error: Too many background processes\n");
                        waitpid(pid, NULL, 0);
                    }
                }
                else
                {
                    int status;
                    waitpid(pid, &status, 0);
                }
            }
        }
    }

    if (input != stdin)
        fclose(input);
    return EXIT_SUCCESS;
}
