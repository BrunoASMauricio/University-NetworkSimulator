#include "receiver.h"


void* receiver(void* _node_id)
{
	int node_id = *((int*)_node_id);
	printf("Started thread dor node %d\n", node_id);
	while(1)
	{
		sleep(1);
	}
}
