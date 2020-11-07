#include "receiver.h"

#define PATH_TO_NODE "../protocol/NP"

void* receiver(void* _node_id)
{
	int node_id;
	pid_t pid;
	
	node_id	= *((int*)_node_id);

	printf("Started receiver thread for node %d\n", node_id);

	pid = fork();

	if(!pid)	// Child (node)
	{
		char pWS[6];
		char pHW[6];
		char pWF_TX[6];
		char pWF_RX[6];
		char pIP[6];

		sprintf(pWS, "%d", S.nodes[node_id].WS->port);
		sprintf(pHW, "%d", S.nodes[node_id].HW->port);
		sprintf(pWF_TX, "%d", S.nodes[node_id].WF_TX->port);
		sprintf(pWF_RX, "%d", S.nodes[node_id].WF_RX->port);
		sprintf(pIP, "%d", S.nodes[node_id].IP);
		printf("%s -q -s --WS %s --HW %s --WF_TX %s --WF_RX %s -IP %s -dlp\n", PATH_TO_NODE, pWS, pHW, pWF_TX, pWF_RX, pIP);
		execl(PATH_TO_NODE, "-s","-q", "--WS", pWS, "--HW", pHW, "--WF_TX", pWF_TX, "--WF_RX", pWF_RX, "--IP", pIP, "-dl", NULL);
		// Only runs if exec fails
		//fatalErr("Could not start node %d\n", node_id);
    }
    else		//Parent (Receiver)
	{
		char buff[MAX_TRANS_SIZE*2];
		int PacketSize;
		void* message;
		int ReadBytes = 0;
		int PrevBytes = 0;
		unsigned long int arrival;

		S.nodes[node_id].process_id = pid;
		printf("Listening on %u\n", S.nodes[node_id].WF_TX->port);
		while(1)
		{
			while((ReadBytes = getFromSocket(S.nodes[node_id].WF_TX, buff+PrevBytes)) == -1)
			{
				continue;
			}
			printf("\nGOT %d bytes. ", ReadBytes);

			if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 144)
			{
				PrevBytes = ReadBytes;
				printf("Got truncated TB. ");
				continue;
			}

			PacketSize = getPacketSize(buff) + 8;	//Expected simulation timestamp

			if(PacketSize == -1)
			{
				dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer: ");
				PrevBytes = 0;
				continue;
			}

			if(PacketSize > ReadBytes + PrevBytes)
			{
				dumpBin(buff, ReadBytes, "Packet size (%d) is more than what is stored(%d). ", PacketSize, ReadBytes + PrevBytes);
				PrevBytes = ReadBytes;
				continue;
			}

			message = malloc(PacketSize-8);
			memcpy(message, buff, PacketSize-8);
			arrival = *((unsigned long int*)(buff+PacketSize-8));
			//printf("Received from node %d with IP %d at %lu: ", S.nodes[node_id].id, S.nodes[node_id].IP, arrival);
			//printMessage(buff, ReadBytes+PrevBytes);


			pthread_mutex_lock(&(S.Lock));
			addToQueue(message, PacketSize-8, S.Sent, arrival);
			pthread_mutex_unlock(&(S.Lock));

			// We received more than one packet
			if(PacketSize < ReadBytes+PrevBytes)
			{
				printf("\t\tMore than one packet (%d, %d), oh no\n", PacketSize, ReadBytes+PrevBytes);
				// Copy the last of the read bytes, to the beggining of the buffer
				PrevBytes = PacketSize;
				for(int i = 0; PacketSize+i < ReadBytes; PacketSize++, i++)
				{
					buff[i] = buff[PacketSize+i];
				}
			}
			PrevBytes = 0;

			//sleep(1);	// For testing purposes
		}
    }
	return NULL;
}
