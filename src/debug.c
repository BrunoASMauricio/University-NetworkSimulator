#include "debug.h"

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

	exit(EXIT_FAILURE);
}

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);

	for(int i = 0; i < size; i++)
    {
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}

	fprintf(stdout, "\n");

	va_end(args);
}

void printNode(int node_id)
{
	printf("\tNode %d\n", node_id);
	for(int other_id = 0; other_id < S.node_ammount; other_id++)
	{
		printf("\t\tReceived SNR from node %d is %d\n", other_id, S.nodes[node_id].SNR[other_id]);
	}
	printf("\tSocket ports:\n");
	printf("\t\tHW: %d\n",S.nodes[node_id].HW->port);
	printf("\t\tWS: %d\n", S.nodes[node_id].WS->port);
	printf("\t\tWF TX: %d\n", S.nodes[node_id].WF_TX->port);
	printf("\t\tWF RX: %d\n", S.nodes[node_id].WF_RX->port);
}

void printNetwork()
{
	printf("Printing network status\n");
	printf("Working with %d nodes. %d is the master node\n", S.node_ammount, S.master);
	for(int node_id = 0; node_id < S.node_ammount; node_id++)
	{
		printNode(node_id);
	}
}

