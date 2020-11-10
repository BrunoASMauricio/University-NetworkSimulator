#include "main.h"



int
main(int argc, char **argv)
{
	int c;
	int rc;
	FILE *f;
	node* n;
	int pid;
	long fsize;
	int node_id;
	char *network;
	char* network_specs;

	S.collision = false;
	S.SNR= false;
	S.main_thread_handle = pthread_self();
	
	while (1)
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"collisions",	no_argument,	0, 'c'},
			{"SNR",		no_argument,		0, 'S'},
			{0,			0,					0,  0 }
		};
		c = getopt_long(argc, argv, "cS", long_options, &option_index);

		if (c == -1)	break;

		switch(c) {
			case 'c':
				S.collision = true;
				break;
			case 'S':
				S.SNR= true;
				break;
		}
	}

	if((S.events = fopen("./0.sim", "w")) == NULL)
	{
		fatalErr("Could not open event log file\n");
	}

    if((f = fopen("networksetup.sim.in", "r")) == NULL)
	{
		fatalErr("Could not open networksetup.sim.in, make sure it exists and has the right read permissions\n");
	}
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	network_specs = (char*)malloc(fsize + 1);
	fread(network_specs, fsize, 1, f);
	fclose(f);

	network_specs[fsize] = 0;

	printf("network specs:\n%s", network_specs);
	// Expected values
	// X	<-- amount of nodes
	// M	<-- Master node (node IDs start at 0)
	// Y0 0	Y0 1	... Y0 X-1
	// Y1 0	Y1 1	... Y1 X-1
	//
	// ...
	//
	// YX-1 YX-1 1	... YX-1 X-1
	//
	//	^
	//	|
	// YA B -> SNR A gets in messages from B

	// Get node ammount and who is the master
	if((rc =sscanf(network_specs,"%d\n%d\n", &(S.node_ammount), &(S.master))) != 2)
	{
		fatalErr("Format issue on the first two lines (%d)\n", rc);
	}

	network_specs = strchr(network_specs, '\n')+1;
	network_specs = strchr(network_specs, '\n')+1;

	S.nodes = (node*)malloc(sizeof(node)*S.node_ammount);

	// Get node SNRs
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		S.nodes[node_id].id = node_id;
		S.nodes[node_id].SNR = (int*)malloc(sizeof(int)*(S.node_ammount));
		S.nodes[node_id].IP = -1;

		for(int other_node = 0; other_node < S.node_ammount; other_node++)
		{
			if((rc = sscanf(network_specs,"%d", &(S.nodes[node_id].SNR[other_node]))) != 1)
			{
				fatalErr("Format issue on line %d\n", 3+node_id);
			}

			if(other_node == node_id && S.nodes[node_id].SNR[other_node] != 0)
			{
				printf("Found incongruence on node %d. Self SNR should be 0\n", node_id);
			}

			

			if(other_node == S.node_ammount - 1)	// Deal with trailing white spaces
			{
				network_specs = strchr(network_specs, '\n')+1;
			}
			else
			{
				network_specs += 2;
			}
		}
	}

	setupNodes();

	printNetwork();

	signal(SIGINT, interruptShutdown);

	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		
		if(rc = pthread_create(&(S.nodes[node_id].rec_thread_handle), NULL, receiver, &(S.nodes[node_id].id)))
		{
			fatalErr("Error: Unable to create thread, %d\n", rc);
		}
		
		if(rc = pthread_create(&(S.nodes[node_id].tra_thread_handle), NULL, transmitter, &(S.nodes[node_id].id)))
		{
			fatalErr("Error: Unable to create thread, %d\n", rc);
		}
		fprintf(S.events, "%d %lu %lu\n", node_id, S.nodes[node_id].rec_thread_handle, S.nodes[node_id].tra_thread_handle);
	}

	sleep(1);

	for(node_id = 0; node_id < S.node_ammount; node_id++)
	{
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
			execl(PATH_TO_NODE,"-q", "-s", "--WS", pWS, "--HW", pHW, "--WF_TX", pWF_TX, "--WF_RX", pWF_RX, "--IP", pIP, "-dl", NULL);
			fatalErr("Could not start node %d\n", node_id);
		}
	}


	simulator();

}

void interruptShutdown(int dummy) {
	printf("Shutting down simulator\n");
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		kill(S.nodes[node_id].process_id, SIGINT);
		close(S.nodes[node_id].HW->s);
		close(S.nodes[node_id].WS->s);
		close(S.nodes[node_id].WF_TX->s);
		close(S.nodes[node_id].WF_RX->s);
	}
	fflush(S.events);
	exit(EXIT_SUCCESS);
}

void setupNodes()
{
	S.Sent= newQueue();
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		S.nodes[node_id].HW = newServerSocket();
		S.nodes[node_id].WS = newServerSocket();
		S.nodes[node_id].WF_TX = newServerSocket();
		S.nodes[node_id].WF_RX = newServerSocket();
		S.nodes[node_id].IP = newIP();
	}

}

int newIP()
{
	static int ip = 0;
	return ++ip;
}

void simulator()
{
	int size;
	void* buf;
	int bufsize;
	timespec Res;
	queue_el* Q_el;
	bool colided;
	queue_el* Target_el;
	unsigned long int Act;
	unsigned long int intersect_t;

	size = 0;

	// Wait for at least a single element in queue
	if(S.collision)
	{
		while(1)
		{
			while(!S.Sent->Size)
			{
				usleep(SIM_DELAY);
			}

			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

			// Wait to check for collisions
			// | S.Sent->Last->Pr < intersect_t ??
			// intersect_t = S.Sent->First->Pr + bufsize*(WF_delay*1000);
			if(S.Sent->First->Pr + SIM_DELAY > Act)
			{
				usleep(SIM_DELAY - ((Act-S.Sent->First->Pr)/1000));
			}

			pthread_mutex_lock(&(S.Lock));
			size = S.Sent->Size;
			printf("\n-------------------------------------\n");
			printf("\t\t %d messages to analyze\n", S.Sent->Size);
			for(int target = 0; target < size; target++)
			{
				// Calculate colision based on sent time + time to send per
				// bit * ammount of bits
				Target_el = getFromQueue(S.Sent, target);
				intersect_t = Target_el->Pr + Target_el->PacketSize*8*(WF_delay*1000);
				//printf("\t::: %lu\n", Target_el->PacketSize*8*(WF_delay*1000));

				printf("Analyzing colisions starting at %lu and ending at %lu\n", Target_el->Pr, intersect_t);
				// Check who intersected with target
				colided = false;
				for(int msg = target+1; msg < size; msg++)
				{
					// Transmission delta intersected
					// (sent before target finished transmission)
					Q_el = getFromQueue(S.Sent, msg);
					if(Q_el->Pr < intersect_t)
					{
						printf("Colided message %lu \n", Q_el->Pr);
						if(Q_el->Packet != NULL)
						{
							printMessage(Q_el->Packet, Q_el->PacketSize);
							free(Q_el->Packet);
							Q_el->Packet = NULL;
						}
						colided = true;
					}
					else
					{
						printf("No collision %lu   %lu\n", Q_el->Pr, intersect_t);
					}
				}

				if(colided && Target_el->Packet != NULL)
				{
					printf("Colided message %lu \n", Target_el->Pr);
					if(Target_el->Packet != NULL)
					{
						printMessage(Target_el->Packet, Target_el->PacketSize);
						free(Target_el->Packet);
						Target_el->Packet = NULL;
					}
				}
				// Assume that, if we receive a message that has been sent AFTER
				// the first one stopped transmission, the first one won't interfere
				// or be interfered by further messages
				pthread_mutex_unlock(&(S.Lock));
			}
			// Check which messages can be forwarded
			pthread_mutex_lock(&(S.Lock));
			for(int target = 0; target < size; target++)
			{
				Target_el = getFromQueue(S.Sent, target);
				intersect_t = Target_el->Pr + Target_el->PacketSize*8*(WF_delay*1000);
				if(intersect_t > S.Sent->Last->Pr)
				{
					buf = popFromQueue(&bufsize, S.Sent, target);
					target -=1;
					size -= 1;
					if(buf != NULL)
					{
						printf("\tMessage sent successfully %p!\n", buf);
						for(int node_id = 0; node_id < S.node_ammount; node_id++)
						{
							addToQueue(buf, bufsize, S.nodes[node_id].Received, 1);
						}
					}
				}
			}
			pthread_mutex_unlock(&(S.Lock));
		}
	}
	else
	{
		while(1)
		{
			while(!S.Sent->Size)
			{
				usleep(SIM_DELAY);
			}
			while(S.Sent->Size)
			{
				buf = popFromQueue(&bufsize, S.Sent, 0);
				printf("\tMessage sent successfully!\n");
				for(int node_id = 0; node_id < S.node_ammount; node_id++)
				{
					addToQueue(buf, bufsize, S.nodes[node_id].Received, 1);
				}
			}
		}
	}
}
