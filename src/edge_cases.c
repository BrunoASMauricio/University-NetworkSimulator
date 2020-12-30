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
	char RX_TX[10];
	char message_type[10];
	while((rc=fscanf(f, "%s %s %lu %lu %u",RX_TX, message_type, &(delay), &(duration), &(act->type))) != EOF && rc == 5)
	{
		if(!strcmp(RX_TX, "RX"))
		{
			act->RX_or_TX = RX;
		}
		else if(!strcmp(RX_TX, "TX"))
		{
			act->RX_or_TX = TX;
		}
		else if(!strcmp(RX_TX, "ANY"))
		{
			act->RX_or_TX = ANY;
		}
		else
		{
			fatalErr("Edge malformed");
		}
		act->message_type = 0;
		for(int i = 0; i < 10; i++)
		{
			if(!strcmp(message_type, packets[i]))
			{
				act->message_type = i+1;
				break;
			}
		}
		if(!act->message_type)
		{
			fatalErr("Wrong message type %s\n", message_type);
		}


		act->delay = delay*(int64_t)1000000000UL;
		act->duration = duration*(int64_t)1000000000UL;
		printf("RX/TX: %s\nMessage Type: %s\nStart: %luns\nDuration: %luns\nAction: %u\n", RX_TX, message_type, act->delay, act->duration, act->type);
		act->started = 0;
		act += sizeof(action*);
		actual_actions++;
		fscanf(f, "\n");
	}
	ret->action_amm = actual_actions;
	return ret;
}


bool actionActive(int node_id)
{
	timespec res;
	unsigned long int cur;
	node* n = &(S.nodes[node_id]);
	edge_case* e = n->Edge;
	if(S.edges && e)
	{
		printf("Action Active? %d %d %d\n",node_id, e->action_amm, e->current);fflush(stdout);
	}
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
		if((cur_act->RX_or_TX == RX && cur_act->message_type == n->last_RX) ||
		   (cur_act->RX_or_TX == TX && cur_act->message_type == n->last_TX) ||
		   (cur_act->RX_or_TX == ANY))
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

