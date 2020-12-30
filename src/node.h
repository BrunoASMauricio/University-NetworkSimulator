#ifndef NODE
#define NODE

char packets[10][4] = {
	"SD",
	"PB",
	"PR",
	"PC",
	"TB",
	"TA",
	"NE",
	"NEP",
	"NER",
	"NEA"
};

void nodeOn(int node_id);

void nodeOff(int node_id);

void updateNodeState(unsigned char a);

#endif
