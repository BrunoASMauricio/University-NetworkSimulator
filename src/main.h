#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "udp.c"
#include "data.h"

#define SAMPLE_SIZE 16

typedef uint8_t byte;

typedef int port;

typedef struct{
	int id;
	int* SNR;
	int IP;
	int process_id;
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
	queue* Sent;
}sim;

sim S;

void setupNodes();

int newIP();

void intHandler(int dummy);

void simulator();

#include "debug.c"
#include "data.c"
#include "receiver.c"

#endif
