#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "msh.h"


/**
 * @brief Print the last error to stdout
 *
 * @return unsigned int EXIT_FAILURE (or 1)
 */
static inline unsigned int msh_error(void)
{
    perror("msh");
    return EXIT_FAILURE;
}

/**
 * @brief Start new processes to run user commands
 *
 * @param args Arguments to pass to the process
 * @param fd The file descriptor to write to, needed for redirection
 * @return int Status code (1)
 */
static int msh_launcher(char **args, const int fd)
{
    /* Duplicate fd to STDOUT and close the original */
    int stdout_cpy;
    if (fd != STDOUT_FILENO) {
        stdout_cpy = dup(STDOUT_FILENO);

        if (dup2(fd, STDOUT_FILENO) == -1)
            return msh_error();
        if (close(fd) == -1)
            return msh_error();
    }

    pid_t pid = fork(), wpid;

    switch (pid) {
    case -1:
        perror("msh");
        break;
    case 0:
        execvp(args[0], args);

        /*
         * A successful child process will never return,
         * so if the program ever gets this far, something
         * has gone wrong.
         */
        exit(msh_error());
    default:;
        int status;
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1)
                exit(msh_error());
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        break;
    }

    /* Reopen STDOUT if there was a redirection */
    if (fd != STDOUT_FILENO) {
        if (dup2(stdout_cpy, STDOUT_FILENO) == -1)
            exit(msh_error());
        if (close(stdout_cpy) == -1)
            exit(msh_error());
    }

    return 1;
}

/**
 * @brief Execute user commands, either processes or builtins
 *
 * @param args Arguments to pass to processes/builtins
 * @param nelems Number of elements in `args`
 * @return int Status code (1)
 */
static int msh_execute(char **args, const unsigned int nelems)
{
    if (args[0] == NULL)
        return 1;

    for (int i = 0; i < msh_num_builtins(); i++)
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);

    /* -1 because we cant redirect if the last arg is the redirection */
    int fd = STDOUT_FILENO;
    for (unsigned int i = 0; i < nelems - 1; i++) {
        if (strcmp(args[i], ">") == 0) {
            fd = creat(args[i + 1], 0777);
            if (fd == -1)
                return msh_error();
            args[i] = '\0';
        }
    }

    return msh_launcher(args, fd);
}

/**
 * @brief Tokenize user input, splitting on whitespace
 *
 * @param line The users input
 * @return char** An array of tokens
 */
static char **msh_split_line(char *line)
{
#define BUFSIZE 64
#define DELIM   " \t\r\n\a"
    unsigned int bufsize = BUFSIZE, position = 0;
    char *token;
    char **tokens = (char **) calloc(bufsize, sizeof(char *));

    if (!tokens) {
        fputs("msh: allocation error", stderr);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL) {
        tokens[position++] = token;

        /* Allocate more memory once all space is used */
        if (position >= bufsize) {
            bufsize += BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens) {
                fputs("msh: reallocation error", stderr);
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIM);
    }

    tokens[position] = '\0';
    return tokens;
}

/**
 * @brief Read user input from stdin
 *
 * @return char* The users input
 */
static char *msh_read_line(void)
{
    char *line     = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin))
            exit(EXIT_SUCCESS);

        exit(msh_error());
    }

    return line;
}

/**
 * @brief The main program loop
 */
static void msh_loop(void)
{
#define ROOT 0
    int status;
    unsigned int argsize;
    char prompt;
    char *line;
    char **args;
    uid_t uid;

    do {
        /* Check if root */
        uid = geteuid();
        if (uid == ROOT)
            prompt = '#';
        else
            prompt = '$';
        printf("%c ", prompt);

        line = msh_read_line();
        args = msh_split_line(line);

        /* Number of arguments */
        argsize = 0;
        while (args[++argsize] != NULL) {}
        status = msh_execute(args, argsize);

        free(line);
        free(args);
    } while (status);
}

int main(void)
{
    msh_loop();
    return EXIT_SUCCESS;
}
