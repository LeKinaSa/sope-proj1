#ifndef DIR_H
#define DIR_H

#include "parsing.h"

#include <stdlib.h>
#include <sys/stat.h>

enum TrimSlashesMode {
    TRIM_ALL,
    TRIM_DUPLICATES
};

/**
 * @brief Trims slashes at the end of a file path
 * @param path  path to trim
 * @param mode  trim all slashes or only duplicate slashes
 */
void trimSlashes(char* path, enum TrimSlashesMode mode);

/**
 * @brief Obtains the path for a subdirectory or file. Dynamically allocates memory
 * for the path which must be freed by the caller.
 * @param path      base path passed as command line argument
 * @param d_name    the value of the d_name string in a dirent struct
 * @returns         the path of the subdirectory or file, or NULL if args were invalid
 */
char* getDirPath(const char* path, const char* d_name);

/**
 * @brief Calculates the size of a file depending on command line arguments.
 * @param args      pointer to struct with command line argument information
 * @param statBuf   pointer to stat struct corresponding to the file
 * @returns         size of the statted file 
 */
size_t calculateSize(const struct CmdArgs* args, const struct stat* statBuf);

#endif // DIR_H