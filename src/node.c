#include "node.h"

void nodeOn(int node_id)
{
	int pid;
	printf("Starting node %d\n", node_id);
	pid = fork();
	if(!pid)
	{
		// use fork and pipes to mimic shell pipe (stdin -> stdout)
		int p2[2];
		pipe(p2);

		if(!fork())		// Child (monitor) -> NPipe
		{
			close(0);		// Close stdin
			dup(p2[0]);		// On fd 0, (first available), set pipe output
			execlp("../MonitorPipe/NPipe", NULL);
			fatalErr("Could not start node %d\n", node_id);
		}
		else			// Parent (HW ans WS simulators + protocol)
		{
			srand(getpid());
			char pWS[6];
			char pHW[6];
			char pWF_TX[6];
			char pWF_RX[6];
			char pIP[6];
			char isMaster[2] = " ";

			//sprintf(pWS, "%d", S.nodes[node_id].WS->port);
			//sprintf(pHW, "%d", S.nodes[node_id].HW->port);
			sprintf(pWS, "%d", range(49153, 65534));
			sprintf(pHW, "%d", range(49153, 65534));

			sprintf(pWF_TX, "%d", S.nodes[node_id].WF_TX->port);
			sprintf(pWF_RX, "%d", S.nodes[node_id].WF_RX->port);
			sprintf(pIP, "%d", (0x82<<8)|S.nodes[node_id].IP);
			if(node_id == S.master)
			{
				isMaster[0] = 'M';
			}
			else

			{
				isMaster[0] = 'S';
			}
			if(S.SimH || S.SimW)
			{
				if(S.master != node_id && S.SimW)		// Non master nodes can only be connected to WS
				{
					if(!fork())
					{
						execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
						fatalErr("Could not start WS sim for node %d\n", node_id);
					}
				}
				else
				{
					if(!fork())	// Child (simulators)
					{
						if(S.SimH ^ S.SimW)	//Only 1 simulator, no need to fork
						{
							// Feed 1 extra argument (they assume [0] is the program name)
							if(S.SimW)	// WS simulator
							{
								execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
								fatalErr("Could not start WS sim for node %d\n", node_id);
							}
							else		// HW simulator
							{
								execlp("../hw_simulator/hw_sim",  pHW, pHW, pIP, NULL);
								fatalErr("Could not start HW sim for node %d\n", node_id);
							}
						}
						else
						{
							if(!fork())	// WS simulator
							{
								execlp("../ws_simulator/ws_sim", pWS, "ws_output", pWS, pIP, NULL);
								fatalErr("Could not start WS sim for node %d\n", node_id);
							}
							else		// HW simulator
							{
								execlp("../hw_simulator/hw_sim",  pHW, pHW, pIP, NULL);
								fatalErr("Could not start HW sim for node %d\n", node_id);
							}
						}
					}
				}
			}
			close(1);		// Close stdout
			dup(p2[1]);		// On fd 1, set pipe input
			execlp("../protocol/NP", "-s", "-r", isMaster, "--WS", pWS, "--HW", pHW, "--WF_TX", pWF_TX, "--WF_RX", pWF_RX, "--IP", pIP, "-d", NULL);
			fatalErr("Could not start node %d\n", node_id);
		}
		fatalErr("Could not start something %d\n", node_id);
	}
	S.nodes[node_id].process_id = pid;
	S.nodes[node_id].up = true;
}


void nodeOff(int node_id)
{
	fprintf(stdout, "Killing node %d %d\n", S.nodes[node_id].process_id, getpid());
	fflush(stdout);
	sleep(1);
	kill(S.nodes[node_id].process_id, SIGINT);
	S.nodes[node_id].up = false;
}

void updateNodeState(unsigned char a)
{

}
