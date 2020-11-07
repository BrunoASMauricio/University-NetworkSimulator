#include "main.h"



int
main(int argc, char **argv)
{
	//events.sim.out
	char *network;
	FILE *f;
	long fsize;
	char* network_specs;
	node* n;
	int rc;

	f = fopen("networksetup.sim.in", "r");
    if(f == NULL)
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

	signal(SIGINT, intHandler);

	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		if(rc = pthread_create(&(S.nodes[node_id].thread_handle), NULL, receiver, &(S.nodes[node_id].id)))
		{
			fatalErr("Error: Unable to create thread, %d\n", rc);
		}
	}

	simulator();

}

void intHandler(int dummy) {
	printf("Shutting down simulator\n");
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		kill(S.nodes[node_id].process_id, SIGINT);
		close(S.nodes[node_id].HW->s);
		close(S.nodes[node_id].WS->s);
		close(S.nodes[node_id].WF_TX->s);
		close(S.nodes[node_id].WF_RX->s);
	}
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
	void* buf;
	int bufsize;
	timespec Res;
	bool colided;
	unsigned long int Act;
	unsigned long int intersect_t;

	while(1)
	{
		// Wait for at least a single element in queue
		while(!S.Sent->Size)
		{
			usleep(SIM_DELAY);
		}

		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

		// Wait to check for collisions
		if(S.Sent->First->Pr + SIM_DELAY > Act)
		{
			usleep(SIM_DELAY - ((Act-S.Sent->First->Pr)/1000));
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
		}

		pthread_mutex_lock(&(S.Lock));

		// Calculate colision based on sent time + time to send per
		// bit * ammount of bits
		intersect_t = S.Sent->First->Pr + bufsize*(WF_delay*1000);
		printf("-------------------------------------\n");
		printf("\t\t %d messages sent\n", S.Sent->Size);

		colided = false;
		printf("Analyzing colisions starting at %lu and ending at %lu\n", S.Sent->First->Pr, intersect_t);
		for(int msg = 1; msg < S.Sent->Size; msg++)
		{
			
			if(getFromQueue(S.Sent, msg) < intersect_t)
			{
				printf("Colided message %lu \n", getFromQueue(S.Sent, msg));
				buf = popFromQueue(&bufsize, S.Sent, msg);
				printMessage(buf, bufsize);
				msg -= 1;
			}else{
				printf("No collision\n");
			}
		}
		if(colided)
		{
			printf("Colided message\n");
			buf = popFromQueue(&bufsize, S.Sent);
			printMessage(buf, bufsize);
		}
		while(S.Sent->Size)
		{
			buf = popFromQueue(&bufsize, S.Sent);
		}
		pthread_mutex_unlock(&(S.Lock));
	}

}
