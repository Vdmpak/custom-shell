#include "customshell.h"

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

// Executes built-in shell commands
void execute_internal_command(char **args)
{
    if (!args[0])
        return;

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
            return;
        }

        if (!validate_directory(args[1]))
            return;

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
    }
    else if (strcmp(args[0], "clr") == 0)
    {
        system("clear");
    }
    else if (strcmp(args[0], "dir") == 0)
    {
        char cmd[MAX_BUFFER] = "ls -al";
        if (args[1])
        {
            if (!validate_directory(args[1]))
                return;
            strncat(cmd, " ", MAX_BUFFER - strlen(cmd) - 1);
            strncat(cmd, args[1], MAX_BUFFER - strlen(cmd) - 1);
        }
        system(cmd);
    }
    else if (strcmp(args[0], "environ") == 0)
    {
        print_environment();
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        normalize_echo_args(args);
    }
    else if (strcmp(args[0], "help") == 0)
    {
        char *root = getenv("CUSTOMSHELL_ROOT");
        if (!root)
        {
            fprintf(stderr, "help: CUSTOMSHELL_ROOT not set\n");
            return;
        }

        char path[MAX_BUFFER];
        size_t path_len = snprintf(path, sizeof(path), "%s/manual/help.txt", root);
        if (path_len >= sizeof(path))
        {
            fprintf(stderr, "help: Path is too long!\n");
            return;
        }

        if (access(path, F_OK) == -1)
        {
            fprintf(stderr, "help: %s: %s\n", path, strerror(errno));
            return;
        }

        char cmd[MAX_BUFFER + 10];
        size_t cmd_len = snprintf(cmd, sizeof(cmd), "more \"%s\"", path);
        if (cmd_len >= sizeof(cmd))
        {
            fprintf(stderr, "help: Command too long\n");
            return;
        }
        system(cmd);
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
    }
    else if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
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

    char buf[MAX_BUFFER], *args[MAX_ARGS];
    while (1)
    {
        if (input == stdin)
            display_prompt();
        if (!fgets(buf, MAX_BUFFER, input))
            break;

        char **arg = args;
        *arg++ = strtok(buf, SEPARATORS);
        while ((*arg++ = strtok(NULL, SEPARATORS)))
            ;

        if (args[0])
            execute_internal_command(args);
    }

    if (input != stdin)
        fclose(input);
    return EXIT_SUCCESS;
}
