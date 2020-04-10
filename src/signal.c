#define _XOPEN_SOURCE 700   // Allows usage of some GNU/Linux standard functions and structures

#include "signal.h"
#include "log.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>


void logSignalReceived(int signo) {
    char* logInfo[1];

    logInfo[0] = malloc(16);
    sprintf(logInfo[0], "%d", signo);

    createLog(RECEIVE_SIGNAL, logInfo, 1);

    free(logInfo[0]);
}

void logSignalSent(int signo, int pid) {
    char* logInfo[2];
    
    logInfo[0] = malloc(16);
    sprintf(logInfo[0], "%d", signo);
    logInfo[1] = malloc(16);
    sprintf(logInfo[1], "%d", pid);

    createLog(SEND_SIGNAL, logInfo, 2);

    free(logInfo[0]);
    free(logInfo[1]);
}

// -------------------------------------------------- //
// ---------------- Signal Handlers ----------------- //
// -------------------------------------------------- //

void sigintHandler() {
    char* firstChildPIDStr = getenv(FIRST_CHILD_PID);
    pid_t firstChildPID = 0;

    if (firstChildPIDStr != NULL) {
        firstChildPID = atoi(firstChildPIDStr);
        killpg(firstChildPID, SIGSTOP);
    }

    const char* message = "\nDo you wish to stop the process (Y/N)? ";

    const size_t MAX_LEN_ANSWER = 50;
    char answer[MAX_LEN_ANSWER];

    while (true) {
        printf("%s", message);
        scanf("%s", answer);
        
        for (size_t i = 0; answer[i] != '\0'; ++i) {
            answer[i] = toupper(answer[i]);
        }

        if (strcmp(answer, "Y") == 0 || strcmp(answer, "YES") == 0) {
            logSignalSent(SIGTERM, getpgid(0));
            if (firstChildPIDStr != NULL)
                killpg(firstChildPID, SIGTERM);
            logAndExit(1);
        }
        else if (strcmp(answer, "N") == 0 || strcmp(answer, "NO") == 0) {
            logSignalSent(SIGCONT, getpgid(0));
            if (firstChildPIDStr != NULL) 
                killpg(firstChildPID, SIGCONT);
            break;
        }
    }
}

void sigtermHandler() {
    logAndExit(1);
}

void parentSignalHandler(int signo) {
    logSignalReceived(signo);

    switch (signo) {
    case SIGINT:
        sigintHandler();
        break;
    case SIGTERM:
        sigtermHandler();
        break;
    default:
        break;
    }
}

void childSignalHandler(int signo) {
    logSignalReceived(signo);

    switch (signo) {
    case SIGTERM:
        sigtermHandler();
        break;
    default:
        break;
    }
}

int registerHandler(bool parent) {
    struct sigaction action;
    action.sa_flags = 0;

    if (parent) {
        action.sa_handler = parentSignalHandler;
    
        if (sigaction(SIGINT, &action, NULL) != 0 ||
            sigaction(SIGTERM, &action, NULL) != 0) {
            perror("sigaction");
            return 1;
        }
    }
    else {
        action.sa_handler = childSignalHandler;
        
        if (sigaction(SIGTERM, &action, NULL) != 0) {
            perror("sigaction");
            return 1;
        }
    }    

    return 0;
}
