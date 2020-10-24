#ifndef DATA
#define DATA

#include <stdint.h>
#include "main.h"
#include "debug.h"
#include "udp.h"

#define UNSET 255
#define DEFAULT_QUEUE_SIZE

typedef uint8_t byte;

/*
 * Data structures
 */

// DO NOT TOUCH THIS STRUCT

typedef struct{
	pthread_t WF_listener_t;
	pthread_t WF_dispatcher_t;
	
    pthread_t WS_listener_t;
	pthread_t WS_dispatcher_t;
	
    pthread_t HW_dispatcher_t;
    pthread_t HW_listener_t;

	socket_s* Input_socket;
	socket_s* Output_socket;
	bool Post;
	bool Debug;
    
    bool WF_TXsimulator;
    bool WF_RXsimulator;
    bool WS_TXsimulator;
    bool HW_RXsimulator;
    bool Interferance;
    int InterferanceLevel;
	
    FILE* Log;
} meta_data;

// TOUCH THESE :)

enum packet_type{
	SD = 1,
	PB,
	PR
};

typedef struct{
	long int Data;
}sample;

/*
 * Queue related data types
 */
enum priority{
	Normal = 0,
	Sync
};


typedef struct{
	void* Packet;
	void* NextEl;
	int PacketSize;
	int Pr;
} queue_el;

typedef struct{
	queue_el* First;
	queue_el* Last;
	pthread_mutex_t Lock;
	int Size;
} queue;

/*
 * Internal queue is only handled by the WS and HW interfaces
 */
typedef struct{
	byte IsMaster;
	queue* OutboundQueue;
	queue* InboundQueue;
	queue* InternalQueue;
	byte IP[2];
	// ...
} node;

/*
 * Returns an initialized queue
 */
queue*
newQueue();

/*
 * Stores the buffer POINTER value "Buffer", and the size "Size"
 * in the queue "Q" according to the priority "Pr".
 * DOES NOT COPY THE DATA IN THE BUFFER, STORES THE RAW POINTER
 */
void
addToQueue(void* Buffer, int Size, queue* Q, int Pr);

/*
 * Pop the next queued packet.
 * Places the size of the packet in *Size
 * Returns the buffers memmory location
 */
void*
popFromQueue(int* Size, queue* Q);

/*
 * Deallocates a queue
 */
void
delQueue(queue* Q);

meta_data Meta;
node Self;

#endif
