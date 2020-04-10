#ifndef SIGNAL_H
#define SIGNAL_H

#define PARENT_SET      "simpleduParentSet"
#define FIRST_CHILD_PID "simpleduFirstChildPID"

#include <stdbool.h>

int registerHandler(bool parent);

#endif // SIGNAL_H