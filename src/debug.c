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
	int hours, minutes, seconds, day, month, year;

	// time_t is arithmetic time type
	time_t now;

	// Obtain current time
	// time() returns the current time of the system as a time_t value
	time(&now);
	struct tm *local = localtime(&now);
	hours = local->tm_hour;          // get hours since midnight (0-23)
	minutes = local->tm_min;         // get minutes passed after the hour (0-59)
	seconds = local->tm_sec;         // get seconds passed after minute (0-59)

	day = local->tm_mday;            // get day of month (1 to 31)
	month = local->tm_mon + 1;       // get month of year (0 to 11)
	year = local->tm_year + 1900;    // get year since 1900

	va_start(args, fmt);
	pthread_mutex_lock(&(S.OutputLock));
	fprintf(stdout, "[%lu] [!] [%02d:%02d:%02d]", se, hours, minutes, seconds);
	vfprintf(stdout, fmt, args);
	pthread_mutex_unlock(&(S.OutputLock));
	va_end(args);
	fflush(stdout);
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
	if(strlen(fmt))
	{
		vfprintf(stdout, fmt, args);
	}

	for(int i = 0; i < size; i++)
    {
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}

	fprintf(stdout, "\n");
	pthread_mutex_unlock(&(S.OutputLock));

	va_end(args);
	fflush(stdout);
}

void printMessage(void* buff, int size)
{
	pthread_mutex_lock(&(S.OutputLock));
	fprintf(stdout, "Message (%d bytes):\n");
	switch (((byte*)buff)[0] & 0x0f)
	{
		case SD:
			fprintf(stdout,"SD\n");
			break;
		case PB:
			fprintf(stdout,"PB\n");
			break;
		case PR:
			fprintf(stdout,"PR\n");
			break;
		case PC:
			fprintf(stdout,"PC\n");
			break;
		case TA:
			fprintf(stdout,"TA\n");
			break;
		case TB:
			fprintf(stdout,"TB\n");
			break;
		case NE:
			fprintf(stdout,"NE\n");
			break;
		case NEP:
			fprintf(stdout,"NEP\n");
			break;
		case NER:
			fprintf(stdout,"NER\n");
			break;
		case NEA:
			fprintf(stdout,"NEA\n");
			break;
	}
	pthread_mutex_unlock(&(S.OutputLock));
	dumpBin((char*)buff, size, "\n\tDump: ");

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

