#include "TX.h"


void*
WF_dispatcher(void* dummy)
{
	printf("WF Dispatcher on\n");
	while(1)
    {
        sleep(1);
    }
}

void
sendMessage(void* msg)
{

}

void*
HW_dispatcher(void*dummy)
{
	printf("HW Dispatcher on\n");
	while(1)
    {
        sleep(1);
    }
}
