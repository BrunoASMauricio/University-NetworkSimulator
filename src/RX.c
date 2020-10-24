#include "RX.h"


void*
WF_listener(void* dummy)
{
	printf("WF Listener on\n");
	while(1){sleep(1);}

}

void*
getMessage()
{
	return NULL;
}

void*
WS_listener(void* dummy)
{
	printf("WS Listener on\n");
	while(1)
    {
        sleep(1);
    }
}

