#define _XOPEN_SOURCE 700   // Allows usage of some GNU/Linux standard functions and structures

#include "log.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>

long programStartMiliseconds = 0;

char * getFilename() {
    return getenv("LOG_FILENAME");
}

char * getSignal(char * signStr) {
    int signo = atoi(signStr);
    char * sign = "";
    switch (signo) {
        case SIGINT:
            sign = "SIGINT";
            break;
        case SIGQUIT:
            sign = "SIGQUIT";
            break;
        case SIGTSTP:
            sign = "SIGTSTP";
            break;
        case SIGCONT:
            sign = "SIGCONT";
            break;
        case SIGKILL:
            sign = "SIGKILL";
            break;
        case SIGSTOP:
            sign = "SIGSTOP";
            break;
        case SIGTERM:
            sign = "SIGTERM";
            break;
        case SIGABRT:
            sign = "SIGABRT";
            break;
        case SIGALRM:
            sign = "SIGALRM";
            break;
        case SIGSEGV:
            sign = "SIGSEGV";
            break;
        case SIGFPE:
            sign = "SIGFPE";
            break;
        case SIGILL:
            sign = "SIGILL";
            break;
        case SIGUSR1:
            sign = "SIGUSR1";
            break;
        case SIGUSR2:
            sign = "SIGUSR2";
            break;
        case SIGCHLD:
            sign = "SIGCHLD";
            break;
        case SIGBUS:
            sign = "SIGBUS";
            break;
        case SIGHUP:
            sign = "SIGHUP";
            break;
        case SIGIO:
            sign = "SIGIO";
            break;
        case SIGPIPE:
            sign = "SIGPIPE";
            break;
        case SIGPROF:
            sign = "SIGPROF";
            break;
        case SIGPWR:
            sign = "SIGPWR";
            break;
        case SIGSTKFLT:
            sign = "SIGSTKFLT";
            break;
        case SIGSYS:
            sign = "SIGSYS";
            break;
        case SIGTRAP:
            sign = "SIGTRAP";
            break;
        case SIGTTIN:
            sign = "SIGTTIN";
            break;
        case SIGTTOU:
            sign = "SIGTTOU";
            break;
        case SIGURG:
            sign = "SIGURG";
            break;
        case SIGVTALRM:
            sign = "SIGVTALRM";
            break;
        case SIGWINCH:
            sign = "SIGWINCH";
            break;
        case SIGXCPU:
            sign = "SIGXCPU";
            break;
        case SIGXFSZ:
            sign = "SIGXFSZ";
            break;
        default:
            sprintf(sign, "%d", signo);
            break;
    }
    return sign;
}

long getMiliseconds() {
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    return (spec.tv_sec * 1e3) + (spec.tv_nsec / 1e6);
}

long getMilisecondsSinceProgramStart() {
    long miliseconds = getMiliseconds();
    long instant;

    if (programStartMiliseconds == 0) {
        programStartMiliseconds = miliseconds;
        instant = 0;
    }
    else {
        instant = miliseconds - programStartMiliseconds;
    }

    return instant;
}

int createLog(enum EventType event, char **info, int infoSize) {
    long instant = getMilisecondsSinceProgramStart();
    pid_t pid = getpid();
    char * filename = getFilename();

    // No file path is set: return early
    if (filename == NULL) {
        return 1;
    }

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0777);

    if (fd == -1) {
        // LOG_FILENAME is not set or is set to an invalid location
        return 1;
    }

    // Get action and info string
    char *action;
    char infoStr[1024];

    switch (event) {
        case CREATE_PROCESS:
            action = "CREATE";
            // Command Line Arguments
            if (infoSize > 0) {
                for (int i = 0; i < infoSize; ++i) {
                    strcat(infoStr, info[i]);
                    strcat(infoStr, " ");
                }
            }
            break;
        case EXIT_PROCESS:
            action = "EXIT";
            // Exit status
            if (infoSize == 1) {
                sprintf(infoStr, "%s", info[0]);
            }
            break;
        case SEND_SIGNAL:
            action = "SEND_SIGNAL";
            // Sent Signal (info[0])
            // Destination PID (info[1])
            if (infoSize == 2) {
                sprintf(infoStr, "%s %s", getSignal(info[0]), info[1]);
            }
            break;
        case RECEIVE_SIGNAL:
            action = "RECV_SIGNAL";
            // Received Signal
            if (infoSize == 1) {
                sprintf(infoStr, "%s", getSignal(info[0]));
            }
            break;
        case WRITE_TO_PIPE:
            action = "SEND_PIPE";
            // Message wrote to pipe
            if (infoSize == 1) {
                sprintf(infoStr, "%s", info[0]);
            }
            break;
        case READ_FROM_PIPE:
            action = "RECV_PIPE";
            // Message read from pipe
            if (infoSize == 1) {
                sprintf(infoStr, "%s", info[0]);
            }
            break;
        case ENTRY_FILE:
            action = "ENTRY";
            // Number of blocks or bytes (info[0])
            // Path (info[1])
            if (infoSize == 2) {
                sprintf(infoStr, "%s %s", info[0], info[1]);
            }
            break;
        default:
            break;
    }

    // Write to File
    char str[2048];
    sprintf(str, "%ld - %d - %s - %s\n", instant, pid, action, infoStr);
    write(fd, str, strlen(str));
    
    // Close File
    close(fd);
    return 0;
}

void logAndExit(int status) {
    char* logInfo[1];

    logInfo[0] = malloc(16);
    sprintf(logInfo[0], "%d", status);
    createLog(EXIT_PROCESS, logInfo, 1);
    free(logInfo[0]);

    exit(status);
}

void logWriteToPipe(const char* message) {
    if (message == NULL) return;

    char* logInfo[1];
    logInfo[0] = malloc(strlen(message) + 1);
    strcpy(logInfo[0], message);
    createLog(WRITE_TO_PIPE, logInfo, 1);
    free(logInfo[0]);
}

void logWriteEntryToPipe(size_t size, const char* path) {
    if (path == NULL) return;

    char* logInfo[1];
    logInfo[0] = malloc(64 + strlen(path));
    sprintf(logInfo[0], "%lu\t%s", size, path);

    createLog(WRITE_TO_PIPE, logInfo, 1);

    free(logInfo[0]);
}

void logReadFromPipe(const char* message) {
    if (message == NULL) return;

    char* logInfo[1];
    logInfo[0] = malloc(strlen(message) + 1);
    strcpy(logInfo[0], message);
    createLog(READ_FROM_PIPE, logInfo, 1);
    free(logInfo[0]);
}

void logEntry(size_t size, const char* path) {
    if (path == NULL) return;

    char* logInfo[2];
    logInfo[0] = malloc(32);
    logInfo[1] = malloc(strlen(path) + 1);
    sprintf(logInfo[0], "%lu", size);
    strcpy(logInfo[1], path);

    createLog(ENTRY_FILE, logInfo, 2);

    free(logInfo[0]);
    free(logInfo[1]);
}