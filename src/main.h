#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>

#include "udp.c"
#include "data.h"

#define SAMPLE_SIZE 16
// Simulatorr delay in us
#define SIM_DELAY 1E6	//1 s
// WF delay per bit in us
//#define WF_delay 10	// 10us/bit
#define WF_delay 60	// 60us/bit

typedef uint8_t byte;

typedef int port;

// Holds the node related information
typedef struct{
	int id;
	int* SNR;
	int IP;
	int process_id;
	pthread_t rec_thread_handle;
	pthread_t tra_thread_handle;
	socket_s* WF_TX;
	socket_s* WF_RX;
	socket_s* HW;
	socket_s* WS;
	queue* Received;
}node;

// Holds simulator information
typedef struct{
	int master;
	node* nodes;
	queue* Sent;
	FILE* events;
	bool collision;
	int node_ammount;
	pthread_mutex_t Lock;
}sim;

sim S;

void
/*
 * Handles the SIGINT signal
 * Shuts down the simulation
 * as well as all node processes
 */
interruptShutdown(int dummy);

void
/*
 * Sets up the node structures
 */
setupNodes();

int
/*
 * Returns a new IP
 * is sequential, beginning at 1
 */
newIP();

void
/*
 * Executes the simulation loop
 * Loop:
 * 		Waits until the first message is old enough
 * 		Performs collision detection and discards colided messages
 */
simulator();

#include "debug.c"
#include "data.c"
#include "receiver.c"
#include "transmitter.c"

#endif
