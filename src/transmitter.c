#include "transmitter.h"

void* transmitter(void* _node_id)
{
	int n;
	pid_t pid;
	inmessage* msg;
	int bufsize;
	int node_id;
	bool will_send;
	
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
			if(msg->node_id != node_id){
				will_send = true;
				if(S.Pbe)
				{
					float ch = randomChance();
					if(ch < S.nodes[node_id].Pbe[msg->node_id]*getPacketSize(msg->buffer))
					{
						printf("PACKET LOST %f %f\n", ch, S.nodes[node_id].Pbe[msg->node_id]);
						printMessage(msg->buffer, getPacketSize(msg->buffer));
						will_send = false;
					}
				}
				if(!will_send)
				{
					continue;
				}

				while((n = sendToSocket(S.nodes[node_id].WF_RX, msg->buffer, bufsize)) == -1)
				{
					continue;
				}
				while(sendToSocket(S.nodes[node_id].WF_RX, &(S.nodes[node_id].Pbe[msg->node_id]), 4) == -1)
				{
					continue;
				}
				printf("\t\t .............Message (%d bytes) sent to node %d, with SNR %u!, %d to go\n",n , node_id, S.nodes[node_id].Pbe[msg->node_id], S.nodes[node_id].Received->Size);
			}
			else
			{
				printf("Ignoring message for node %d\n", node_id);
			}
		}
		sleep(1);
	}
}
