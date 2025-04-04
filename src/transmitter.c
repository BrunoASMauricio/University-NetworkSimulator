#include "transmitter.h"

void* transmitter(void* _node_id)
{
	int Received;
	pid_t pid;
	inmessage* msg;
	int bufsize;
	int node_id;
	bool will_send;
	action* cur_act;
	
	node_id	= *((int*)_node_id);
	node* n = &(S.nodes[node_id]);
	n->Received= newQueue();
	printf("Started transmitter thread for node %d on port %u\n", node_id, S.nodes[node_id].WF_RX->port);
	char testbuf[10];

	
	// This is needed to kickstart the connection
	// Everywhere I looked, client always spoke first
	// Always found "in UDP it doesn't matter who talks first"
	// But if the client doesn't send a ping, it doesn't work
	/*
	while((n = getFromSocket(S.nodes[node_id].WF_RX, testbuf)) == -1)
	{
		continue;
	}
	*/
	
	while(1)
	{
		usleep(10);
		while(S.nodes[node_id].Received->Size)
		{
			msg = (inmessage*)popFromQueue(&bufsize, S.nodes[node_id].Received);

			if(1 || msg->node_id != node_id){
				if(S.edges && n->Edge && n->Edge->current != n->Edge->action_amm)
				{
					cur_act = &(n->Edge->actions[n->Edge->current]);
					if(actionActive(node_id) && CHECKBIT(Deafen, cur_act->type))
					{
						printf("Deafening message for %d\n", node_id);
						printMessage(msg->buffer, msg->size);
						// Delete message later
						continue;
					}
				}
				will_send = true;
				if(S.Pbe)
				{
					double ch = randomChance();
					double p = 1-pow(1-S.nodes[node_id].Pbe[msg->node_id], getPacketSize(msg->buffer));
					printf("Chances from node %d to node %d\n%f %f (%d), ch = %f\n",
							msg->node_id,
							node_id,
							S.nodes[node_id].Pbe[msg->node_id],
							p,
							getPacketSize(msg->buffer), ch);
					printf("%f %f\n", 1-S.nodes[node_id].Pbe[msg->node_id], p);
					if(ch < p)
					{
						printf("PACKET LOST %f %f from node %d to node %d\n", ch, S.nodes[node_id].Pbe[msg->node_id], msg->node_id, node_id);
						printMessage(msg->buffer, getPacketSize(msg->buffer));
						will_send = false;
					}
					else
					{
						printf("PACKET NOT LOST!\n");
					}
				}
				if(!will_send)
				{
					continue;
				}

				while((Received = sendToSocket(S.nodes[node_id].WF_RX, msg->buffer, bufsize)) == -1)
				{
					continue;
				}
				float pbe = S.nodes[node_id].Pbe[msg->node_id];
				/*
				static float i = 0;
				float rdc = randomChance();
				if(rdc < 0.2){
					rdc = randomChance();
					pbe += 0.0001;
				}
				*/
				if(!pbe){
					pbe = 0.0000001;
				}
				while(sendToSocket(S.nodes[node_id].WF_RX, &(pbe), 4) == -1)
				{
					continue;
				}
				dumpBin((char*)(msg->buffer), bufsize, "\t\t .............Message (%d/%d bytes) sent from node %d to node %d, with SNR %f!, %d to go\n",Received, bufsize,msg->node_id, node_id, S.nodes[node_id].Pbe[msg->node_id], S.nodes[node_id].Received->Size);

				if(S.edges)
				{
					n->last_RX = ((byte*)(msg->buffer))[0] & 0x0f;
					actionActive(node_id);
				}

			}
			else
			{
				printf("Ignoring message for node %d\n", node_id);
			}
		}
	}
}
