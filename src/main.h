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
// Simulatorr delay in us
#define SIM_DELAY 1E6	//1 s
// WF delay per bit in us
//#define WF_delay 10	// 10us/bit
#define WF_delay 60000

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
	int master;
	node* nodes;
	queue* Sent;
	FILE* events;
	int node_ammount;
	pthread_mutex_t Lock;
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
