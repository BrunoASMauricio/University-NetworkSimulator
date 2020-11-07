#include "receiver.h"

#define PATH_TO_NODE "../protocol/NP"

void* receiver(void* _node_id)
{
	int node_id;
	pid_t pid;
	
	node_id	= *((int*)_node_id);

	printf("Started thread for node %d\n", node_id);

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
		printf("%s -s --WS %s --HW %s --WF_TX %s --WF_RX %s -IP %s -dlp\n", PATH_TO_NODE, pWS, pHW, pWF_TX, pWF_RX, pIP);
		execl(PATH_TO_NODE, "-s", "--WS", pWS, "--HW", pHW, "--WF_TX", pWF_TX, "--WF_RX", pWF_RX, "--IP", pIP, "-dl", NULL);
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
			printf("\nGOT %d bytes\n", ReadBytes);

			if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 144)
			{
				PrevBytes = ReadBytes;
				printf("Got truncated TB\n");
				continue;
			}

			PacketSize = getPacketSize(buff) + 8;	//Expected simulation timestamp

			if(PacketSize == -1)
			{
				dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer\n");
				PrevBytes = 0;
				continue;
			}

			if(PacketSize > ReadBytes)
			{
				dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize-8, ReadBytes);
				PrevBytes = ReadBytes;
				continue;
			}
			printf("Received ");
			switch (((byte*)buff)[0])
			{
				case SD:
					printf("SD");
					break;
				case PB:
					printf("PB");
					break;
				case PR:
					printf("PR");
					break;
				case PC:
					printf("PC");
					break;
				case TA:
					printf("TA");
					break;
				case TB:
					printf("TB");
					break;
				case NE:
					printf("NE");
					break;
				case NEP:
					printf("NEP");
					break;
				case NER:
					printf("NER");
					break;
				case NEA:
					printf("NEA");
					break;
			}
			printf("\n");
			dumpBin(buff, ReadBytes+PrevBytes, "dump: ");
			fflush(stdout);

			message = malloc(PacketSize-8);
			memcpy(message, buff, PacketSize-8);
			arrival = *((unsigned long int*)(buff+PacketSize-8));
			printf(" message from node %d with IP (%d) received at %lu\n", S.nodes[node_id].id, S.nodes[node_id].IP, arrival);
			addToQueue(message, PacketSize, S.Sent, arrival);
			printf("Messages in queue %d\n", S.Sent->Size);

			// We received more than one packet
			if(PacketSize < ReadBytes)
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

			sleep(1);	// For testing purposes
		}
        waitpid(pid,0,0); /* wait for child to exit */
    }
	return NULL;
}
