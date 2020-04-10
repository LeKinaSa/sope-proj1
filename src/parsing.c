#include "parsing.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define LOG_FILENAME "LOG_FILENAME="

//https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c
bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

bool directoryExists(const char *path) {
    DIR* dir = opendir(path);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return true;
    } else {
        /* opendir() failed for some reason. */
        return false;
    }
}

bool simpleOption(const char *inputString,
                  const char *shortName,
                  const char *longName,
                  bool *option) {

    bool shortMatches = strcmp(inputString, shortName) == 0;
    bool longMatches  = strcmp(inputString, longName)  == 0;

    if (shortMatches || longMatches) {
        *option = true;
        return true;
    } else {
        return false;
    }
}

struct CmdArgs parseArgs(int argc, char* argv[], char *envp[]) {
    struct CmdArgs res;
    res.path         = NULL; 
    res.logFile      = NULL;
    res.all          = false;  
    res.bytes        = false;
    res.countLinks   = false; 
    res.dereference  = false; 
    res.separateDirs = false;
    res.blockSize    = 0;
    res.maxDepth     = -1; // This will underflow to the max value

    if (argc < 2 || argc > 10) {
        printf("Usage: %s -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n", argv[0]);
        exit(1);
    }

    for (int i = 0; ; i++) {
        // If a file path is not set
        if (envp[i] == NULL) {
            res.logFile = NULL;
            break;
        }

        if (startsWith(LOG_FILENAME, envp[i])) {
            int strSize = strlen(envp[i]) - strlen(LOG_FILENAME) + 1;
            res.logFile = (char *) malloc(strSize);

            strcpy(res.logFile, envp[i] + strlen(LOG_FILENAME));
        }
    }

    for (int i = 1; i < argc;){

        if (simpleOption(argv[i], "-b", "--bytes", &res.bytes)) {
            res.blockSize = 1;
            ++i;
            continue;
        }

        if (simpleOption(argv[i], "-a", "--all",           &res.all)         ||
            simpleOption(argv[i], "-l", "--count-links",   &res.countLinks)  ||
            simpleOption(argv[i], "-L", "--dereference",   &res.dereference) ||
            simpleOption(argv[i], "-S", "--separate-dirs", &res.separateDirs)
        ) {
            ++i;
            continue;
        }

        if (!startsWith("-", argv[i])) {
            if (res.path == NULL) {
                if (directoryExists(argv[i])) {
                    int strSize = strlen(argv[i]) + 1;
                    res.path = (char *) malloc(strSize);
                    strcpy(res.path, argv[i]);
                } else {
                    fprintf(stderr, "Error: \"%s\" is not a valid directory path\n", argv[i]);
                    exit(1);
                }
            } else {
                fprintf(stderr, "Error: More than one possible path\n");
                exit(1);
            }
            i++;
            continue;
        }

        if (strcmp("-B", argv[i]) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "Error: \"-B\" option requires a numerical argument\n");
                exit(1);
            } else {
                bool success = sscanf(argv[i + 1], "%u", &res.blockSize) == 1;
                if (!success) {
                    fprintf(stderr, "Error: \"-B\" option requires a numerical argument\n");
                    exit(1);
                }
            }
            i += 2;
            continue;
        }

        if (startsWith("--block-size=", argv[i])) {
            bool success = sscanf(argv[i] + strlen("--block-size="), "%u", &res.blockSize) == 1;
            if (!success) {
                fprintf(stderr, "Error: In \"--block-size=SIZE\", SIZE must be an unsigned integer\n");
                exit(1);
            }
            ++i;
            continue;
        }

        if (startsWith("--max-depth=", argv[i])) {
            bool success = sscanf(argv[i] + strlen("--max-depth="), "%u", &res.maxDepth) == 1;
            if (!success) {
                fprintf(stderr, "Error: In \"--max-depth=N\", N must be an unsigned integer\n");
                exit(1);
            }
            ++i;
            continue;
        }


        fprintf(stderr, "Error: option \"%s\" not recognized\n", argv[i]);
        exit(1);
    }

    if (!res.countLinks) {
        fprintf(stderr, "Error: \"-l\" option must be set\n");
        exit(1);
    }

    if (res.path == NULL) {
        res.path = ".";
    }
    
    if (res.blockSize == 0) {
        res.blockSize = 1024;
    }

    return res;
}

void printArgs(const struct CmdArgs *args) {
    printf("Args debug:       \n");
    printf("    path:         %s\n",args->path);
    printf("    all:          %s\n",args->all          ? "true" : "false");
    printf("    bytes:        %s\n",args->bytes        ? "true" : "false");
    printf("    blockSize:    %u\n",args->blockSize);
    printf("    countLinks:   %s\n",args->countLinks   ? "true" : "false");
    printf("    dereference:  %s\n",args->dereference  ? "true" : "false");
    printf("    separateDirs: %s\n",args->separateDirs ? "true" : "false");
    printf("    maxDepth:     %u\n",args->maxDepth);
    printf("    logFile:      %s\n",args->logFile);
}

char **genChildArgv(int argc, char* argv[], const char *dirChildPath) {
    // https://stackoverflow.com/questions/36804759/how-to-copy-argv-to-a-new-variable
    char** newArgv = malloc((argc + 1) * sizeof(char*));
    for(int i = 0; i < argc; ++i) {
        size_t length = strlen(argv[i]) + 1;
        newArgv[i] = malloc(length);
        memcpy(newArgv[i], argv[i], length);
    }
    newArgv[argc] = NULL;

    bool pathFound = false;

    for (int i = 1; i < argc; ++i){

        if (strcmp("-B", newArgv[i]) == 0) {
            i++;
            continue;
        }

        if (!startsWith("-", newArgv[i])) { // It's the path
            pathFound = true;
            free(newArgv[i]);
            
            newArgv[i] = malloc(strlen(dirChildPath) + 1);
            strcpy(newArgv[i], dirChildPath);

            continue;
        }

        if (startsWith("--max-depth=", newArgv[i])) {
            unsigned int depth;
            bool success = sscanf(newArgv[i] + strlen("--max-depth="), "%u", &depth) == 1;
            if (!success) {
                fprintf(stderr, "genChildrenArgv error: In \"--max-depth=N\", N must be an unsigned integer\n");
                exit(1);
            }

            if (depth != 0) {
                depth--;

                char *newString = malloc(1024 * sizeof(char));
                sprintf(newString, "--max-depth=%u", depth);

                free(newArgv[i]);
                newArgv[i] = newString;
            }

            continue;
        }
    }

    if (!pathFound) {
        ++argc; // We need space for the path
        newArgv = realloc(newArgv, (argc + 1) * sizeof (char*));
        newArgv[argc] = NULL;

        int strSize = strlen(dirChildPath) + 1;
        newArgv[argc - 1] = malloc(strSize);
        strcpy(newArgv[argc - 1], dirChildPath);
    }

    return newArgv;
}
