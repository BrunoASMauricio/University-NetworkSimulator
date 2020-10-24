#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include "main.h"

#define printf printfLog

void
/*
 * print a message, and dump a binary
 */
dumpBin(char* buf, int size, const char *fmt,...);

void
/*
 * Normal output
 */
printfLog(const char *fmt, ...);

void
/*
 * Error output
 */
printfErr(const char *fmt, ...);

void
/*
 * Try to print to a file
 * Print to stderr
 * End program with EXIT_FAILURE
 */
fatalErr(const char *fmt, ...);

void
/*
 * Test the queue manipulation functions
 */
testQueues();

void
/*
 * Perform network/CPU measurements
 * */
performMeasurements();

void
/*
 * Performs a Self Test
 * When a problem can be detected, will halt the program
 * and return an error
 */
testAll();


#endif
