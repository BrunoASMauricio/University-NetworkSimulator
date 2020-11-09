#ifndef DATA
#define DATA

#include "main.h"

typedef struct{
	void* Packet;
	void* NextEl;
	int PacketSize;
	unsigned long int Pr;
} queue_el;

typedef struct{
	queue_el* First;
	queue_el* Last;
	pthread_mutex_t Lock;
	int Size;
} queue;

const int Packet_Sizes[11] = {-1, 56/8, 56/8, 80/8, 64/8, 144/8, 56/8, 40/8, 40/8, 40/8, 40/8};

enum packet_type{
	SD = 1,
	PB,
	PR,
	PC,
	TB,
	TA,
	NE,
	NEP,
	NER,
	NEA
};

queue*
newQueue();

void
addToQueue(void* Packet, int Size, queue* Q, unsigned long int Pr);

void*
popFromQueue(int* Size, queue* Q);

void*
/*
 * Pops element at given position
 */
popFromQueue(int* Size, queue* Q, int position);

queue_el*
/*
 * Returns the queue_el at the requested position
 */
getFromQueue(queue* Q, int position);

int
getPacketSize(void* buf);



#endif
