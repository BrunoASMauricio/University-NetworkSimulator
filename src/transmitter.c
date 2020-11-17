#include "transmitter.h"

void* transmitter(void* _node_id)
{
	int n;
	pid_t pid;
	inmessage* msg;
	int bufsize;
	int node_id;
	
	node_id	= *((int*)_node_id);
	S.nodes[node_id].Received= newQueue();
	printf("Started transmitter thread for node %d on port %u\n", node_id, S.nodes[node_id].WF_RX->port);
	char testbuf[10];

	
	// This is needed to kickstart the connection
	// Everywhere I looked, client always spoke first
	// Always found "in UDP it doesn't matter who talks first"
	// But if the client doesn't send a ping, it doesn't work
	while((n = getFromSocket(S.nodes[node_id].WF_RX, testbuf)) == -1)
	{
		continue;
	}

	while(1)
	{
		while(S.nodes[node_id].Received->Size)
		{
			msg = (inmessage*)popFromQueue(&bufsize, S.nodes[node_id].Received);
			while((n = sendToSocket(S.nodes[node_id].WF_RX, msg->buffer, bufsize)) == -1)
			{
				continue;
			}
			while(sendToSocket(S.nodes[node_id].WF_RX, &(S.nodes[node_id].SNR[msg->node_id]), 2) == -1)
			{
				continue;
			}
			printf("\t\t .............Message (%d bytes) sent to node %d, with SNR %u!, %d to go\n",n , node_id, S.nodes[node_id].SNR[msg->node_id], S.nodes[node_id].Received->Size);
		}
		sleep(1);
	}
}
