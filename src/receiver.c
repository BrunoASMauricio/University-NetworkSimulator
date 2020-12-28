#include "receiver.h"

#define PATH_TO_NODE "../protocol/NP"

void* receiver(void* _node_id)
{
	int node_id;
	pid_t pid;
	action* cur_act;
	
	node_id	= *((int*)_node_id);

	char buff[MAX_TRANS_SIZE*2];
	int PacketSize;
	inmessage* message;
	int ReadBytes = 0;
	//int PrevBytes = 0;
	unsigned long int arrival;
	node* n = &(S.nodes[node_id]);

	n->process_id = pid;
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
		while((ReadBytes = getFromSocket(n->WF_TX, buff)) == -1)
		{
			continue;
		}
		assert(ReadBytes == 8);
		arrival = *((unsigned long int*)(buff));
		// Check action
		if(n->Edge && n->Edge->current != n->Edge->action_amm)
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

		//updateNodeState(((byte*)message->buffer)[0] & 0x0f);

		pthread_mutex_lock(&(S.Lock));
		addToQueue(message, message->size, S.Sent, arrival);
		pthread_mutex_unlock(&(S.Lock));
	}
	return NULL;
}
