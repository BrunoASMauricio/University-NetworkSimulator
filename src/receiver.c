#include "receiver.h"

#define PATH_TO_NODE "../protocol/NP"

void* receiver(void* _node_id)
{
	int node_id;
	pid_t pid;
	
	node_id	= *((int*)_node_id);

	char buff[MAX_TRANS_SIZE*2];
	int PacketSize;
	inmessage* message;
	int ReadBytes = 0;
	//int PrevBytes = 0;
	unsigned long int arrival;

	S.nodes[node_id].process_id = pid;
	printf("Started receiver thread for node %d on %u\n", node_id, S.nodes[node_id].WF_TX->port);
	while(1)
	{
		while((ReadBytes = getFromSocket(S.nodes[node_id].WF_TX, buff)) == -1)
		{
			continue;
		}
		message = (inmessage*)malloc(sizeof(message));

		message->buffer = malloc(ReadBytes);
		memcpy(message->buffer, buff, ReadBytes);
		message->node_id = node_id;
		message->size = ReadBytes;

		// Read timestamp
		while((ReadBytes = getFromSocket(S.nodes[node_id].WF_TX, buff)) == -1)
		{
			continue;
		}
		assert(ReadBytes == 8);
		arrival = *((unsigned long int*)(buff));
		
		printf("Received from node %d with IP %d at %lu: ", S.nodes[node_id].id, S.nodes[node_id].IP, arrival);
		printMessage(message->buffer, message->size);


		pthread_mutex_lock(&(S.Lock));
		addToQueue(message, message->size, S.Sent, arrival);
		pthread_mutex_unlock(&(S.Lock));
	}
	return NULL;
}
