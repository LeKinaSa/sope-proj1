#define _GNU_SOURCE

#include "parsing.h"
#include "dir.h"
#include "log.h"
#include "signal.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <string.h>
#include <math.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

int main(int argc, char *argv[], char *envp[]) {
    createLog(CREATE_PROCESS, argv, argc);

    bool parent = (getenv(PARENT_SET) == NULL);

    if (parent) {
        setenv(PARENT_SET, "True", 0);
    }
    else {
        char* value = getenv(FIRST_CHILD_PID);

        if (value == NULL) {
            // First child
            char pidStr[16];
            sprintf(pidStr, "%d", getpid());

            setenv(FIRST_CHILD_PID, pidStr, 0);

            if (setpgid(0, 0) != 0)
                perror("setpgid");
        }
        else {
            if (setpgid(0, atoi(value)) != 0)
                perror("setpgid");
        }
    }

    registerHandler(parent);

    struct CmdArgs args = parseArgs(argc, argv, envp);

    trimSlashes(args.path, TRIM_DUPLICATES);

    // Directory stream related setup

    DIR *dirPtr;
    struct dirent *direntPtr;
    struct stat statBuf;

    if ((dirPtr = opendir(args.path)) == NULL) {
        perror(args.path);
        exit(1);
    }

    int pipeFD[2];

    char* dirPath;

    size_t currentDirSize = 0;

    while ((direntPtr = readdir(dirPtr)) != NULL) {
        if (strcmp(direntPtr->d_name, "..") == 0) {
            continue;
        }

        dirPath = getDirPath(args.path, direntPtr->d_name);

        if (args.dereference) {
            if (stat(dirPath, &statBuf) != 0) {
                perror("stat failed");
            }
        }
        else {
            if (lstat(dirPath, &statBuf) != 0) {
                perror("lstat failed");
            }
        }
      
        if (strcmp(direntPtr->d_name, ".") == 0) {
            currentDirSize += calculateSize(&args, &statBuf);
            continue;
        }

        if (S_ISDIR(statBuf.st_mode)) {
            if (pipe(pipeFD) < 0) {
                perror("pipe");
                exit(1);
            }

            char** childArgv = genChildArgv(argc, argv, dirPath);

            pid_t pid = fork();

            if (pid > 0) { // Parent
                close(pipeFD[PIPE_WRITE]);
                int status;

                waitpid(pid, &status, 0);
            }
            else if (pid == 0) { // Child
                close(pipeFD[PIPE_READ]);

                // Make child standard output point to the pipe's write end
                if (dup2(pipeFD[PIPE_WRITE], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(1);
                }
              
                execv(argv[0], childArgv);
                perror("execv");
                exit(1);
            }
            else { // Error occurred
                perror("fork");
            }
          
            // Create FILE pointer from file descriptor to use in getline()
            FILE* fp = fdopen(pipeFD[PIPE_READ], "r");

            while (true) {
                char* line = NULL;
                size_t len;

                int retVal = getline(&line, &len, fp);

                if (retVal <= 0) {
                    free(line);
                    break;
                }

                size_t subDirSize;
                char subDirPath[256];  // Maybe change this later and somehow use memory allocation?

                sscanf(line, "%lu%s", &subDirSize, subDirPath);

                if (!args.separateDirs && strcmp(dirPath, subDirPath) == 0) {
                    currentDirSize += subDirSize;
                }

                if (args.maxDepth > 0) {
                    if (strcmp(dirPath, subDirPath) == 0) {
                        printf("%lu\t%s\n", (size_t)(ceil((double)(subDirSize) / args.blockSize)), subDirPath);
                    }
                    else {
                        printf("%s", line);
                    }
                }
                

                free(line);
            }

            close(pipeFD[PIPE_READ]);

            size_t i = 0;
            while (childArgv[i] != NULL) {
                free(childArgv[i++]);
            }
            free(childArgv);

            free(dirPath);
            continue;
        }

        size_t fileSize = calculateSize(&args, &statBuf);
        currentDirSize += fileSize;

        if (args.all && args.maxDepth > 0) {
            printf("%lu\t%s\n", (size_t)(ceil((double)(fileSize) / args.blockSize)), dirPath);
        }

        free(dirPath);
    }

    size_t sizeToPrint = parent ? ceil((double)(currentDirSize) / args.blockSize) : currentDirSize;
    printf("%lu\t%s\n", sizeToPrint, args.path);

    char *exit_info[1] = {"0"};
    createLog(EXIT_PROCESS, exit_info, 1);
    return 0;
}
