#include "data.h"

queue*
newQueue()
{
	queue* Q = (queue*)malloc(sizeof(queue));
	Q->Size = 0;
	Q->First = NULL;
	Q->Last= NULL;
	
    if (pthread_mutex_init(&(Q->Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for queue lock\n");
    }
	return Q;
}

/*
 * The reason for using linked lists instead of arrays
 * Is to allow for prioritized packets to "jump ahead" easily
 * If the memory allocation proves to be too slow or non-deterministic,
 * a pool of pre-allocated queue_el can be implemented
 */
void
addToQueue(void* Packet, int Size, queue* Q, unsigned long int Pr)
{
	queue_el* NewEl = (queue_el*)malloc(sizeof(queue_el));
	queue_el *LastHigher;
	queue_el *ToSwap;

	NewEl->Packet = Packet;
	NewEl->PacketSize = Size;
	NewEl->Pr = Pr;
	NewEl->NextEl = NULL;

	pthread_mutex_lock(&(Q->Lock));
	ToSwap = Q->First;
	LastHigher = Q->First;

	while(ToSwap != NULL && ToSwap->Pr >= Pr)
    {
		LastHigher = ToSwap;
		ToSwap = (queue_el*)ToSwap->NextEl;
	}

	if(Q->First == NULL)
    {
        Q->First = NewEl;
		Q->Last = NewEl;
	}
    else
    {
		if(ToSwap == LastHigher)
        {
			NewEl->NextEl= LastHigher;
			Q->First = NewEl;
		}
        else
        {
			if(ToSwap == NULL)
            {
				Q->Last = NewEl;
			}
			NewEl->NextEl = LastHigher->NextEl;
			LastHigher->NextEl = NewEl;
		}
	}
	Q->Size += 1;
	pthread_mutex_unlock(&(Q->Lock));
}


void*
popFromQueue(int* Size, queue* Q)
{	
	queue_el* Popped;
	void* Buf;
	
	if (Q->Size == 0)
    {
		return 0;
	}

	pthread_mutex_lock(&(Q->Lock));
	Popped = Q->First;
	Q->First = (queue_el*)Q->First->NextEl;
	Q->Size -= 1;
	pthread_mutex_unlock(&(Q->Lock));

	Buf = Popped->Packet;
	*Size = Popped->PacketSize;

	free(Popped);

	return Buf;
}

void
delQueue(queue* Q)
{
	pthread_mutex_destroy(&(Q->Lock));
	free(Q);
}

int
getPacketSize(void* buf)
{
	byte type;
	int helper;

   	type = ((char*)buf)[0] & 0x0f;
	
	switch(type)
	{
		case SD:
			helper = ((char*)buf)[6]*SAMPLE_SIZE;
			if(8*(helper/8) != helper)
			{
				helper = helper/8 +1;
			}
			else
			{
				helper /= 8;
			}
			return Packet_Sizes[SD] + helper;
		case TB:
			helper = ((short*)buf)[8];
			if(8*(helper/8) != helper)
			{
				helper = helper/8 +1;
			}
			else
			{
				helper /= 8;
			}
			return Packet_Sizes[TB] + ((short*)buf)[8]*2 + helper;
		default:
			if(type > sizeof(Packet_Sizes) -1)
			{
				printf("Unrecognized Message type %d\n", type);
				return -1;
			}
			return Packet_Sizes[type];
	}
}
