# Simulator

The WF simulator.

Simulates message exchange between nodes.

Can simulate message loss, temporal collision and transmission jitter (NOT IMPLEMENTED).

Can force edge cases to happen by performing user-defined message blocking actions, based on perceived protocol state and the given time intervals.

Reads network.sim.in to setup nodes and their communications.

Reads X.sim.in to retrieve actions to perform on node X.


network.sim.in

X							<-- amount of nodes

M							<-- Master node (node IDs start at 0)

Y0,0	Y0,1	... Y0,X-1

Y1,0	Y1,1	... Y1,X-1

...

YX-1,0 YX-1,1	... YX-1,X-1

YA,B -> Bit error probability from a message sent from B, and received by A
Note:
	The Yk,k values should always be 0 (node receives all it's sent messages), since this is a worst case scenario.

X.sim.in

STATE0	X0 Y0

STATE1	X1 Y1

...

SATEK	XK YK

STATE = 0 (Outside) | 1 (Waiting) | 2 (Inside)
X = Delay from when the state is first reached, to when the blocking action begins in seconds
Y = Duration of the blocking action in seconds

The way the simulator interprets the file, is sequentially.

The simulator waits until the first action elapses, to execute the second one.

This means that if the first actions' state is Inside, and the second actions' state is Outside, the simulator will wait until the node is Inside, perform the action for its' duration, and then wait until the node is considered Outside once again. This may never happen unless other actions on other nodes make it so.


Code parameters:

    --WS/-w :	Use WS simulator for all nodes

	--HW/-h :	Use HW simulator for the Master node

	--collisions/-c :	Simulates message colision
						"distance" between nodes does not affect this
						We always simulate the worst case scenario, where any temporal overlap in message transmission leads to all interfering messages being lost for all nodes

	--pbe/-p :	Uses the network.sim.in to retrieve the Bit Error Probability from each node, to each other node
		
	--jitter/-j:	Adds the default jitter to the communications (NOT IMPLEMENTED)

	--edges/-e:		Makes the simulator enforce the rules on the given X.sim.in files


