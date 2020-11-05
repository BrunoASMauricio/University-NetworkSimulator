#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "udp.c"

typedef uint8_t byte;

typedef int port;

typedef struct{
	int id;
	int* SNR;
	int IP;
	pthread_t thread_handle;
	socket_s* WF_TX;
	socket_s* WF_RX;
	socket_s* HW;
	socket_s* WS;
}node;

typedef struct{
	FILE* events;
	int node_ammount;
	int master;
	node* nodes;
}sim;

sim S;

void setupNodes();

#include "debug.c"
#include "receiver.c"

#endif
