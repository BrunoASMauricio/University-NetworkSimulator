#include "debug.h"

void
printfLog(const char *fmt, ...)
{
	va_list args;
	pthread_t se = pthread_self();
	if(se == S.main_thread_handle)
	{
	}
	/*
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	*/
	va_start(args, fmt);
	pthread_mutex_lock(&(S.OutputLock));
	fprintf(stdout, "[%lu] [!]", se);
	vfprintf(stdout, fmt, args);
	pthread_mutex_unlock(&(S.OutputLock));
	va_end(args);
}
/*
void
printfErr(const char *fmt, ...)
{
	va_list args;
	pthread_t se = pthread_self();
	if(se == S.main_thread_handle)
	{
	}
	fprintf(stdout, "[%lu] [X]", se);
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fflush(stdout);
}
*/

void
fatalErr(const char *fmt, ...)
{
	FILE* t;
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "[XX]");
    vfprintf(stderr, fmt, args);
	fflush(stderr);
    va_end(args);
	fflush(stderr);

	exit(EXIT_FAILURE);
}

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
	pthread_t se = pthread_self();

	va_start(args, fmt);

	pthread_mutex_lock(&(S.OutputLock));
	fprintf(stdout, "[%lu] [!]", se);
	vfprintf(stdout, fmt, args);

	for(int i = 0; i < size; i++)
    {
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}

	fprintf(stdout, "\n");
	pthread_mutex_unlock(&(S.OutputLock));

	va_end(args);
}

void printMessage(void* buff, int size)
{
	printf("Message (%d bytes):\n");
	switch (((byte*)buff)[0] & 0x0f)
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
	dumpBin((char*)buff, size, "\n\tDump: ");
	printf("\n");
	fflush(stdout);
}

void printNode(int node_id)
{
	printf("\tNode %d (IP: %d)\n", node_id, S.nodes[node_id].IP);
	for(int other_id = 0; other_id < S.node_ammount; other_id++)
	{
		printf("\t\tReceived SNR from node %d is %f\n", other_id, S.nodes[node_id].Pbe[other_id]);
	}
	printf("\tSocket ports:\n");
	//printf("\t\tHW: %d\n",S.nodes[node_id].HW->port);
	//printf("\t\tWS: %d\n", S.nodes[node_id].WS->port);
	printf("\t\tWF TX: %d\n", S.nodes[node_id].WF_TX->port);
	printf("\t\tWF RX: %d\n", S.nodes[node_id].WF_RX->port);
}

void printNetwork()
{
	printf("Printing network status (%lu):\n",getpid());
	printf("Working with %d nodes. %d is the master node\n", S.node_ammount, S.master);
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		printNode(node_id);
	}
	printf("Simulation status:\n");
	printf("Using collisions: %d\n", S.collision);
}

