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


#include "edge_cases.h"

#define SAMPLE_SIZE 16
// Simulator delay in us
//#define SIM_DELAY (unsigned int)1E5	//0.01 s -> 100.000 us
#define SIM_DELAY (unsigned int)1E4	//0.001 s -> 10.000 us
//#define WF_delay 10	// 10us/bit
// WF delay per bit in ns
#define WF_delay ((unsigned long int)(3*1E4)) // ~30us/bit

// Jitter in ms
#define WF_JITTER 30	// 30ms

//maximum message delay inside the simulator in ns
#define MAX_PACKET_SEND_DELAY 1E6

typedef uint8_t byte;

//typedef int port;

// Holds the node related information
typedef struct{
	int id;
	float* Pbe;
	int IP;
	int process_id;
	pthread_t rec_thread_handle;
	pthread_t tra_thread_handle;
	socket_s* WF_TX;
	socket_s* WF_RX;
	socket_s* HW;
	socket_s* WS;
	queue* Received;
	edge_case* Edge;
	byte last_RX;
	byte last_TX;
	bool up;
}node;

// Holds simulator information
typedef struct{
	int master;
	node* nodes;
	queue* Sent;
	FILE* events;
	bool collision;
	bool edges;
	bool SimH;
	bool SimW;
	bool Pbe;
	bool jitter;
	int node_ammount;
	pthread_t main_thread_handle;
	pthread_mutex_t Lock;
	pthread_mutex_t OutputLock;
}sim;

typedef struct{
	void* buffer;
	int node_id;
	int size;
}inmessage;

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
#include "edge_cases.c"
#include "node.c"

#endif
