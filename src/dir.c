#include "dir.h"

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ST_BLOCKS_SIZE 512

void trimSlashes(char* path, enum TrimSlashesMode mode) {
    if (path == NULL) return;
    if (mode != TRIM_ALL && mode != TRIM_DUPLICATES) return;
    
    int i = strlen(path) - 1;

    while (i >= 0 && path[i] == '/')
        i--;

    // Only removes duplicate dashes, hence the i + 2
    if (i != (int)(strlen(path) - 1)) {
        if (mode == TRIM_ALL)
            path[i + 1] = 0;
        else
            path[i + 2] = 0;
    }
}

char* getDirPath(const char* path, const char* d_name) {
    if (path == NULL || d_name == NULL) return NULL;

    char* temp = malloc(strlen(path) + 1);
    strcpy(temp, path);
    trimSlashes(temp, TRIM_ALL);

    char* dirPath = malloc(strlen(temp) + strlen(d_name) + 2);
    sprintf(dirPath, "%s%s%s", temp, "/", d_name);

    free(temp);

    return dirPath;
}

size_t calculateSize(const struct CmdArgs* args, const struct stat* statBuf) {
    return args->bytes ? statBuf->st_size : statBuf->st_blocks * ST_BLOCKS_SIZE;
}