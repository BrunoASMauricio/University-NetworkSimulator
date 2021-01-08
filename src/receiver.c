#include "receiver.h"

#define PATH_TO_NODE "../protocol/NP"

void* receiver(void* _node_id)
{
	int node_id;
	action* cur_act;
	
	node_id	= *((int*)_node_id);

	char buff[MAX_TRANS_SIZE*2];
	int PacketSize;
	inmessage* message;
	int ReadBytes = 0;
	int timestamp_size;
	//int PrevBytes = 0;
	unsigned long int arrival;
	unsigned long int last_arrival = 0;
	int last_read_Bytes;
	node* n = &(S.nodes[node_id]);

	printf("Started receiver thread for node %d on %u\n", node_id, n->WF_TX->port);
	while(1)
	{
		while((ReadBytes = getFromSocket(n->WF_TX, buff)) == -1)
		{
			continue;
		}
		message = (inmessage*)malloc(sizeof(message));

		message->buffer = malloc(ReadBytes);
		memcpy(message->buffer, buff, ReadBytes);
		message->node_id = node_id;
		message->size = ReadBytes;

		// Read timestamp
		while((timestamp_size = getFromSocket(n->WF_TX, buff)) == -1)
		{
			continue;
		}
		if(timestamp_size != 8)
		{
			dumpBin(buff, ReadBytes, "YEEE %d %d\n", node_id, timestamp_size);fflush(stdout);
			printf("Ye\n");
			fflush(stdout);
		}
		assert(timestamp_size == 8);
		arrival = *((unsigned long int*)(buff));
		
		if(S.jitter)
		{
			// New message arrived before the last one finished being sent
			printf("before %lu\n", arrival);
			if(last_arrival != 0 && last_arrival+WF_delay*last_read_Bytes*8 > arrival)
			{
				arrival = (unsigned long int)(last_arrival+WF_delay*last_read_Bytes*8+(unsigned long int)(range(0,WF_JITTER)*1000000));
			}
			else
			{
				arrival += (unsigned long int)(range(0,WF_JITTER)*1000000);
			}
			printf("after %lu\n", arrival);
		}

		updateNodeState(((byte*)message->buffer)[0] & 0x0f);

		// Check action
		if(S.edges && n->Edge && n->Edge->current != n->Edge->action_amm)
		{
			cur_act = &(n->Edge->actions[n->Edge->current]);
			if(actionActive(node_id) && CHECKBIT(Mute, cur_act->type))
			{
				printf("Muting message for %d\n", node_id);
				printMessage(message->buffer, message->size);
				// Delete message later
				continue;
			}
		}
		printf("Received from node %d with IP %d at %lu: ", S.nodes[node_id].id, S.nodes[node_id].IP, arrival);
		printMessage(message->buffer, message->size);
		if(S.edges)
		{
			n->last_TX = ((byte*)(message->buffer))[0] & 0x0f;
			actionActive(node_id);
		}

		last_read_Bytes = ReadBytes;
		last_arrival = arrival;
		pthread_mutex_lock(&(S.Lock));
		addToQueue(message, message->size, S.Sent, arrival);
		pthread_mutex_unlock(&(S.Lock));
	}
	return NULL;
}
