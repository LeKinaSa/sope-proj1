#ifndef PARSING_H
#define PARSING_H

#include <stdbool.h>

/**
 * @brief All the cmd and envp arguments in one place
 *
 * If a bool is true, it means that option was set (e.g. countLinks is always true)
 */
struct CmdArgs {
    char *path; /** The relative path */
    bool all;  /** Write counts for all files, not just directories */
    bool bytes; /** Display the real number of used (files) or allocated (folders) bytes (this option probably overrides -B) */
    unsigned int blockSize; /** Use SIZE-byte blocks */
    bool countLinks; /** Count multiple times the same file (THIS OPTION IS ALWAYS SET) */
    bool dereference; /** Follow all symbolic links */
    bool separateDirs; /** Do not include size of subdirectories */
    unsigned int maxDepth; /** The recursion limit */

    char *logFile; /** The log file path */
};

/**
 * @brief Parses all the arguments into a neat struct
 *
 * This function WILL CRASH if invalid options are parsed
 *
 * @param argc
 * @param argv
 * @param envp
 * @return struct CmdArgs
 */
struct CmdArgs parseArgs(int argc, char* argv[],  char *envp[]);

/**
 * @brief Prints all the values of the struct
 *
 * @param args
 */
void printArgs(const struct CmdArgs *args);

/**
 * @brief Generates a completely new argv, with the dirChildPath appended to path and,
 *        if --max-depth is set, decrement it's value.
 *
 * @param argc
 * @param argv
 * @param dirChildPath
 * @return char**
 */
char **genChildArgv(int argc, char* argv[], const char *dirChildPath);

#endif // PARSING_H
