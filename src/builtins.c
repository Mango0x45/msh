#include <stdio.h>
#include <unistd.h>

#include "msh.h"


char *builtin_str[]            = {"cd", "exit", "help"};
int (*builtin_func[])(char **) = {&msh_cd, &msh_exit, &msh_help};

inline int msh_num_builtins(void)
{
    return sizeof(builtin_str) / sizeof(char *);
}

int msh_cd(char **args)
{
    if (args[1] == NULL)
        fputs("msh: expected arguement to 'cd'", stderr);
    else if (chdir(args[1]) == -1)
        perror("msh");

    return 1;
}

int msh_exit(char **UNUSED(args))
{
    return 0;
}

int msh_help(char **UNUSED(args))
{
    puts("Mango Shell (msh)\n"
         "v1.0\n"
         "The following is a list of all the shells current builtins\n");

    for (int i = 0; i < msh_num_builtins(); i++)
        printf("  %s\n", builtin_str[i]);

    return 1;
}