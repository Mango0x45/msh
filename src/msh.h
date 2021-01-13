#ifndef __MSH_H_
#define __MSH_H_

/* Suppress unused parameter warnings */
#ifdef __GNUC__
#    define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#    define UNUSED(x) UNUSED_##x
#endif

extern char *builtin_str[];
extern int (*builtin_func[])(char **);

/**
 * @brief Get the number of msh builtins
 *
 * @return int The number of builtins
 */
int msh_num_builtins(void);

/**
 * @brief Change directories
 *
 * @param args The directory to change into
 * @return int Status code (1)
 */
int msh_cd(char **args);

/**
 * @brief Exit the shell
 *
 * @param args Unused
 * @return int Status code (0)
 */
int msh_exit(char **args);

/**
 * @brief Display a help message
 *
 * @param args Unused
 * @return int Status code (1)
 */
int msh_help(char **args);

#endif