# Simulator

The WF simulator.

Simulates message exchange between nodes.

Can simulate message loss and temporal collision

Reads network.sim.in to setup nodes and their communications.

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


Code parameters:

    --WS/-w :	Use WS simulator for all nodes

	--HW/-h :	Use HW simulator for the Master node

	--collisions/-c :	Simulates message colision
						"distance" between nodes does not affect this
						We always simulate the worst case scenario, where any temporal overlap in message transmission leads to all interfering messages being lost for all nodes

	--pbe/-p :	Uses the network.sim.in to retrieve the Bit Error Probability from each node, to each other node
		


