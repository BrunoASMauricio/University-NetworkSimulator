#include "transmitter.h"

void* transmitter(void* _node_id)
{
	int n;
	pid_t pid;
	void* buf;
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
			buf = popFromQueue(&bufsize, S.nodes[node_id].Received);
			while((n = sendToSocket(S.nodes[node_id].WF_RX, buf, bufsize)) == -1)
			{
				continue;
			}
			printf("\t\t .............Message (%d bytes) sent to node!, %d to go\n", n, S.nodes[node_id].Received->Size);
		}
		sleep(1);
	}
}
