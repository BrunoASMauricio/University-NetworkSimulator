#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

#define SETBIT(bit,byte) byte |= (1 << bit)
#define CLEARBIT(bit,byte) byte &= ~(1 << bit)
#define CHECKBIT(bit,byte) (byte) & (1 << bit)

#define printf printfLog

void
fatalErr(const char *fmt, ...);

void
dumpBin(char* buf, int size, const char *fmt,...);

void
printMessage(void* buff, int size);

void
/*
 * Prints the node setup
 */
printNode(int node_id);

void
/*
 * Outputs the network setup
 */
printNetwork();
#endif
