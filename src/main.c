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
	S.jitter = false;
	S.Pbe = false;
	S.SimH = false;
	S.SimW = false;
	S.main_thread_handle = pthread_self();
	srand(time(0));
	
	while (1)
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"HW",			no_argument,		0, 'h'},
			{"WS",			no_argument,		0, 'w'},
			{"collisions",	no_argument,		0, 'c'},
			{"jitter",		no_argument,		0, 'j'},
			{"pbe",			no_argument,		0, 'p'},
			{0,				0,					0,  0 }
		};
		c = getopt_long(argc, argv, "cjp", long_options, &option_index);

		if (c == -1)	break;

		switch(c) {
			case 'c':
				S.collision = true;
				break;
			case 'j':
				S.jitter = true;
				break;
			case 'p':
				S.Pbe = true;
				break;
			case 'h':
				S.SimH = true;
				break;
			case 'w':
				S.SimW = true;
				break;
		}
	}

	
	int p2[2];
	pipe(p2);

	
	pid = fork();
	if(!pid)		// Child (monitor) -> NPipe
	{
		close(0);		// Close stdin
		dup(p2[0]);		// On fd 0, (first available), set pipe output
		execlp("../MonitorPipe/NPipe", NULL);
		fatalErr("There was a problem with the simulators' NPipe\n");
	}
	
	close(1);		// Close stdout
	dup(p2[1]);		// On fd 1, set pipe input
	fprintf(stdout, "0\n");
	fprintf(stdout, "%d\n", getpid());
	fflush(stdout);
	

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

	printf("Simulators:\n");
	printf("WF = 1 (always)\n");
	printf("HW = %d\n", S.SimH);
	printf("WS = %d\n", S.SimW);

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
		S.nodes[node_id].Pbe= (float*)malloc(sizeof(float)*(S.node_ammount));
		S.nodes[node_id].IP = -1;

		for(int other_node = 0; other_node < S.node_ammount; other_node++)
		{
			if((rc = sscanf(network_specs,"%f", &(S.nodes[node_id].Pbe[other_node]))) != 1)
			{
				fatalErr("Format issue on line %d\n", 3+node_id);
			}

			if(other_node == node_id && S.nodes[node_id].Pbe[other_node] != 0)
			{
				printf("Found incongruence on node %d. Self Pbe should be 0\n", node_id);
			}

			if(other_node == S.node_ammount - 1)	// Deal with trailing white spaces
			{
				network_specs = strchr(network_specs, '\n')+1;
			}
			else
			{
				network_specs = strchr(network_specs, ' ')+1;
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
		fprintf(stdout, "[] %d %lu %lu\n", node_id, S.nodes[node_id].rec_thread_handle, S.nodes[node_id].tra_thread_handle);
	}

	//sleep(1);
	//pipe(p1);
	for(node_id = 0; node_id < S.node_ammount; node_id++)
	{
		if(!fork())
		{
			printf("Starting node %d\n", node_id);
			// use fork and pipes to mimic shell pipe (stdin -> stdout)
			int p2[2];
			pipe(p2);

			pid = fork();
			if(!pid)		// Child (monitor) -> NPipe
			{
				close(0);		// Close stdin
				dup(p2[0]);		// On fd 0, (first available), set pipe output
				execlp("../MonitorPipe/NPipe", NULL);
				fatalErr("Could not start node %d\n", node_id);
			}
			else			// Parent (HW ans WS simulators + protocol)
			{
				srand(getpid());
				char pWS[6];
				char pHW[6];
				char pWF_TX[6];
				char pWF_RX[6];
				char pIP[6];
				char isMaster[2] = " ";

				S.nodes[node_id].process_id = pid;

				//sprintf(pWS, "%d", S.nodes[node_id].WS->port);
				//sprintf(pHW, "%d", S.nodes[node_id].HW->port);
				sprintf(pWS, "%d", 23092);//range(49153, 65534));
				sprintf(pHW, "%d", range(49153, 65534));

				sprintf(pWF_TX, "%d", S.nodes[node_id].WF_TX->port);
				sprintf(pWF_RX, "%d", S.nodes[node_id].WF_RX->port);
				sprintf(pIP, "%d", (0x82<<8)|S.nodes[node_id].IP);
				if(node_id == S.master)
				{
					isMaster[0] = 'M';
				}
				else
				{
					isMaster[0] = 'S';
				}
				if(S.SimH || S.SimW)
				{
					if(S.master != node_id && S.SimW)		// Non master nodes can only be connected to WS
					{
						if(!fork())
						{
							execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
							fatalErr("Could not start WS sim for node %d\n", node_id);
						}
					}
					else
					{
						if(!fork())	// Child (simulators)
						{
							if(S.SimH ^ S.SimW)	//Only 1 simulator, no need to fork
							{
								// Feed 1 extra argument (they assume [0] is the program name)
								if(S.SimW)	// WS simulator
								{
									execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
									fatalErr("Could not start WS sim for node %d\n", node_id);
								}
								else		// HW simulator
								{
									execlp("../hw_simulator/hw_sim",  pHW, pHW, pIP, NULL);
									fatalErr("Could not start HW sim for node %d\n", node_id);
								}
							}
							else
							{
								if(!fork())	// WS simulator
								{
									execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
									fatalErr("Could not start WS sim for node %d\n", node_id);
								}
								else		// HW simulator
								{
									execlp("../hw_simulator/hw_sim",  pHW, pHW, pIP, NULL);
									fatalErr("Could not start HW sim for node %d\n", node_id);
								}
							}
						}
					}
				}
				close(1);		// Close stdout
				dup(p2[1]);		// On fd 1, set pipe input
				execlp("../protocol/NP", "-s", "-r", isMaster, "--WS", pWS, "--HW", pHW, "--WF_TX", pWF_TX, "--WF_RX", pWF_RX, "--IP", pIP, "-d", NULL);
				fatalErr("Could not start node %d\n", node_id);
			}
			fatalErr("Could not start something %d\n", node_id);
		}
	}
	fflush(stdout);
	simulator();
}

void interruptShutdown(int dummy) {
	if(pthread_self() != S.main_thread_handle)
	{
		return;
	}
	printf("Shutting down simulator\n");
	// Kill whole process group. Not sure if this works
	//kill(0,SIGINT);
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		printf("Shutting down node %d\n", node_id);
		kill(S.nodes[node_id].process_id, SIGINT);
		//close(S.nodes[node_id].HW->s);
		//close(S.nodes[node_id].WS->s);
		close(S.nodes[node_id].WF_TX->s);
		close(S.nodes[node_id].WF_RX->s);
	}
	fflush(stdout);
	fflush(stderr);
	exit(EXIT_SUCCESS);
}

void setupNodes()
{
	S.Sent= newQueue();
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		//S.nodes[node_id].HW = newServerSocket();
		//S.nodes[node_id].WS = newServerSocket();
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
			printf("\n-------------------------------------\n");
			size = S.Sent->Size;
			printf("\t\t %d messages to analyze\n", size);
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
						if(Q_el->Packet != NULL)
						{
							printf("Colided message %lu \n", Q_el->Pr);
							printMessage(Q_el->Packet, Q_el->PacketSize);
							free(((inmessage*)(Q_el->Packet))->buffer);
							((inmessage*)(Q_el->Packet))->buffer = NULL;
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

				if(colided)
				{
					if(Target_el->Packet != NULL && ((inmessage*)(Target_el->Packet))->buffer != NULL)
					{
						printf("Colided message %lu \n", Target_el->Pr);
						printMessage(((inmessage*)(Target_el->Packet))->buffer, Target_el->PacketSize);
						free(((inmessage*)(Target_el->Packet))->buffer);
						((inmessage*)(Target_el->Packet))->buffer = NULL;
						free(Target_el->Packet);
						Target_el->Packet = NULL;

					}
				}
			}
			// Check which messages can be forwarded
			// Assume that, if we receive a message that has been sent AFTER
			// the first one stopped transmission, the first one won't interfere
			// or be interfered by further messages
			printf("\t\t %d messages to collide\n", S.Sent->Size);
			for(int target = 0; target < size; target++)
			{
				Target_el = getFromQueue(S.Sent, target);
				intersect_t = Target_el->Pr + Target_el->PacketSize*8*(WF_delay*1000);
				if(intersect_t > S.Sent->Last->Pr || intersect_t > MAX_PACKET_SEND_DELAY)
				{
					buf = popFromQueue(&bufsize, S.Sent, target);
					target -=1;
					size -= 1;
					if(buf != NULL)
					{
						dumpBin((char*)((inmessage*)buf)->buffer, bufsize, "\tMessage sent successfully!\n");
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
				dumpBin((char*)((inmessage*)buf)->buffer, bufsize, "\tMessage sent successfully!\n");
				for(int node_id = 0; node_id < S.node_ammount; node_id++)
				{
					addToQueue(buf, bufsize, S.nodes[node_id].Received, 1);
				}
			}
		}
	}
}
