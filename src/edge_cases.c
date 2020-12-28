#include "edge_cases.h"

edge_case* getEdgeActions(int node_id)
{
	int rc;
	FILE* f;
	char ch;
	int lines;
	action* act;
	int actual_actions;
	unsigned int delay;
	unsigned int duration;
	char file_name[20];
	edge_case* ret = (edge_case*)malloc(sizeof(edge_case));

	sprintf(file_name, "%d.sim.in", node_id);
    if((f = fopen(file_name, "r")) == NULL)
	{
		printf("No edge rules (%s) for node %d\n", file_name, node_id);
		return NULL;
	}
	while((ch=fgetc(f))!=EOF)
	{
		if(ch=='\n')
		{
			lines++;
		}
	}
	fseek(f, 0, SEEK_SET);
	ret->actions = (action*)malloc(sizeof(action)*lines);
	act = ret->actions;
	actual_actions = 0;
	printf("Rules:\n");
	while((rc=fscanf(f, "%d %lu %lu %u", &(act->state), &(delay), &(duration), &(act->type))) != EOF && rc == 4)
	{
		act->delay = delay*(int64_t)1000000000UL;
		act->duration = duration*(int64_t)1000000000UL;
		printf("State: %d\nStart: %lus\nDuration: %lus\nAction: %u\n", act->state, act->delay, act->duration, act->type);
		act->started = 0;
		act += sizeof(action*);
		actual_actions++;
		fscanf(f, "\n");
	}
	return ret;
}


bool actionActive(int node_id)
{
	timespec res;
	unsigned long int cur;
	node* n = &(S.nodes[node_id]);
	printf("Action Active? %d\n",node_id);fflush(stdout);
	edge_case* e = n->Edge;
	// No actions left (ended or never where)
	if(!S.edges || !e || e->action_amm == e->current)
	{
		return false;
	}

	action* cur_act = &(e->actions[e->current]);

	// Analyze action based on node state
	// Start action?
	if(!(cur_act->started))
	{
		printf("Action not started\n");fflush(stdout);
		if(cur_act->state == n->state || cur_act->state == -1)
		{
			printf("Starting action\n");fflush(stdout);
			clock_gettime(CLOCK_REALTIME, &res);
			cur_act->started = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
		}
		else
		{
			return false;
		}
	}

	clock_gettime(CLOCK_REALTIME, &res);
	cur = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
	// Action already started ?
	// On a separate if to handle "delay = 0"
	if(cur_act->started && cur > cur_act->started+cur_act->delay)
	{
		printf("Action started\n");
		// End action?
		if(cur > cur_act->started+cur_act->delay+cur_act->duration)
		{
			printf("Action ended\n");
			if(CHECKBIT(Shutdown, cur_act->type))
			{
				nodeOn(node_id);
			}
			e->current += 1;
			return false;
		}
		else
		{
			printf("Message blocked by rule %d\n",e->current);
			if(S.nodes[node_id].up && CHECKBIT(Shutdown, cur_act->type))
			{
				nodeOff(node_id);
			}

			return true;
		}
	}
	return false;
}

