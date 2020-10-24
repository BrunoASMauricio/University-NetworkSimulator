#include "debug.h"

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
	if (Meta.Log)
    {
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		for(int i = 0; i < size; i++)
        {
			fprintf(Meta.Log, "0x%02hhx ", buf[i]);
		}
		fprintf(Meta.Log, "\n");
		va_end(args);
	}
}

void
printfLog(const char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
	va_end(args);
	if (Meta.Log) {
		fprintf(Meta.Log, "[!] ");
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		va_end(args);
		//fflush(Meta.Log);
	}
	fflush(stdout);
}


void
printfErr(const char *fmt, ...)
{
	if (!Meta.Debug) return;
	va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
	if (Meta.Log) {
		fprintf(Meta.Log, "[X] ");
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		va_end(args);
		//fflush(Meta.Log);
	}
}


void
fatalErr(const char *fmt, ...)
{
	FILE* t;
	va_list args;
	if((t = fopen("./fatal", "w")) != NULL) 
    {
		va_start(args, fmt);
		vfprintf(t, fmt, args);
		fflush(t);
		va_end(args);
	}
    else
    {
		fprintf(stderr, "Could not create \"fatal\" file\n");
	}
	va_start(args, fmt);
	fprintf(stderr, "[XX]");
    vfprintf(stderr, fmt, args);
	fflush(stderr);
    va_end(args);
	clean();
	exit(EXIT_FAILURE);
}

void
testQueues()
{
	queue* aq;
	char buff1[24] = "bean good, how bout you";
	char buff2[19] = "hello how you bean";
	char buff3[18] = "very nice, thanks";
	char buff4[14] = "just a packet";
	int dummy;
	char* dummy2;

	aq	= newQueue();
	addToQueue((void*)buff4, strlen(buff4), aq, -2);
	addToQueue((void*)buff3, strlen(buff3), aq, -1);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff2, strlen(buff2), aq, 10);
	addToQueue((void*)buff4, strlen(buff4), aq, -2);

	printf("Expected output: %s %s %s\n",  buff2, buff2, buff2);
	printf("%s\n%s\n%s\n%s\n%s\n%s\n\n",
					buff2, buff1, buff1,
					buff3, buff4, buff4);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n\n", dummy2);

	delQueue(aq);	
}

void
performMeasurements(){
	struct timespec res;
	struct timespec res2;

	long avg = 0;
	long total = 0;
	for(int i = 0; i < 100; i++)
	{
		clock_gettime(CLOCK_REALTIME, &res);
		clock_gettime(CLOCK_REALTIME, &res2);
		avg += ((res2.tv_sec - res.tv_sec) * (int64_t)1000000000UL) + (res2.tv_nsec - res.tv_nsec);
		total++;
		// sleep(1); <-- this changes the average, why
	}
	printf("clock_gettime(CLOCK_REAL_TIME) average sampling delay: %lld\n", avg/total);
}

void
testAll(){
	char a[6];
	a[0] = 0xaf;
	a[1] = 0x99;
	a[2] = 0x82;
	a[3] = 0xff;
	a[4] = 0x00;
	a[5] = 0x11;

	printf("Starting protocol test\n---------\n");

	printf("Testing printfLog. Expected output:\n");
	printf(">>1 teste 0.5\n");
	printf(">>%d teste %0.1f\n", 1, 0.5);

	printf("Testing dumpBin. Expected output:\n");
	printf(">>Hello 4: 0xaf 0x99 0x82 0xff 0x00 0x11\n");
	
	dumpBin(a, sizeof(a), ">>Hello %d: ", 4);

	printf("Testing queue data structures\n");

	testQueues();





	printf("Ending protocol test\n---------\n");
	printf("Starting protocol measurements\n---------\n");
	performMeasurements();
	printf("Ending protocol measurements\n---------\n");


}
