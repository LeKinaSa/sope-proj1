#ifndef LOG_H
#define LOG_H

#include <stdlib.h>

/**
 * @brief All the event types available for logging
 */
enum EventType {
    CREATE_PROCESS,     // info = command line arguments    | size = n
    EXIT_PROCESS,       // info = exit code                 | size = 1
    SEND_SIGNAL,        // info = signal + pid              | size = 2
    RECEIVE_SIGNAL,     // info = received signal           | size = 1
    WRITE_TO_PIPE,      // info = sent message              | size = 1
    READ_FROM_PIPE,     // info = received message          | size = 1
    ENTRY_FILE          // info = bytes/blocks + path       | size = 2
};

/**
 * @brief Write the log information to the file (LOG_FILENAME)
 * 
 * @param event
 * @param info
 * @param infoSize
 *  
 * @return int
 */
int createLog(enum EventType event, char **info, int infoSize);

/** @brief Logs an EXIT_PROCESS entry and exits with the given status */
void logAndExit(int status);

/** @brief Logs a WRITE_TO_PIPE entry with the given message */
void logWriteToPipe(const char* message);

/** @brief Logs a WRITE_TO_PIPE entry in the format <size>\t<path> */
void logWriteEntryToPipe(size_t size, const char* path);

/** @brief Logs a READ_FROM_PIPE entry with the given message */
void logReadFromPipe(const char* message);

/** @brief Logs an ENTRY_FILE entry with the given entry info */
void logEntry(size_t size, const char* path);

#endif // LOG_H