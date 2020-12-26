#include "edge_cases.h"

edge_case* getEdgeRules(int node_id)
{
	char file_name[20];
	FILE* f;
	sprintf(file_name, "%d.sim.in", node_id);
    if((f = fopen(file_name, "r")) == NULL)
	{
		printf("No edge rules for node %d\n", node_id);
		return NULL;
	}
	return NULL;
}

