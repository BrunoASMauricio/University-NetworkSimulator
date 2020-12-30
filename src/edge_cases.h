#ifndef EDGE
#define EDGE

#include "main.h"
#include "node.h"

#define RX 0
#define TX 1
#define ANY 2

enum state{
	Outside = 0,
	Waiting,
	Inside
};

enum action_type{
	Deafen = 0,		// Node does not receive messages while the action is active
	Mute,			// Node does not transmit messages while the action is active
	Shutdown,		// Node is offline while the action is active
};

/*
 * How many (start) s after switching to a
 * state (state), should the messages be blocked,
 * and after how many (end) s to stop
 */
typedef struct{
	unsigned long int delay;			// Time from entering stage, and engaging action
	unsigned long int duration;		// Time from start of action, to end
	int RX_or_TX;					// On RX (0) or TX (1) or ANY (2)
	int message_type;				// The message type. Is ignored on type == ANY
	unsigned long int started;
	action_type type;
}action;

typedef struct{
	action* actions;
	int current;
	int action_amm;
}edge_case;

/*
 * Read edge actions from file and into
 * edge_case
 */
edge_case* getEdgeActions(int node_id);

/*
 * Check if an action is active
 * true if yes
 * false if no
 */
bool actionActive(int node_id);

#endif
