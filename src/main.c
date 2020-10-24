#include "main.h"
#include "TX.h"
#include "data.h"
#include "simulator.c"
#include <bits/getopt_core.h>

/*
 * Main function
 * Parses the command line arguments (sets verbosity and debugging)
 * Starts the main protocol loop
 */
int
main(int argc, char **argv)
{
	int c;

	Meta.Debug = false;
	Meta.Post= false;
	Meta.Log = NULL;
	Self.IsMaster = UNSET;

	while (1) 
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"log",		no_argument,		0, 'l'},
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{0,			0,					0,  0 }
		};

		c = getopt_long(argc, argv, "lpdrs:", long_options, &option_index);

		if (c == -1)	break;

		switch(c) {
			case 'r':
				if (optarg[0] == 'M') 
                {
					Self.IsMaster = true;
					printf("Forcing node to master\n");
				}
                else if (optarg[0] == 'S')
                {
					Self.IsMaster = false;
					printf("Forcing node to slave\n");
				}
                else
                {
					fatalErr("Node can only be master (M) or slave (S)\n");
				}
				break;

			case 'l':
				if((Meta.Log = fopen("./log", "a")) == NULL)
                {
					fatalErr("Could not open log. Errno set to: %d\n", errno);
				}
				break;
			
            case 's':
                if(optarg[0] == 'W' && optarg[1] == 'S')
                {
				    Meta.WS_TXsimulator= true;
                    printf("Simulating WS TX\n");
                }
                else if( optarg[0] == 'W' && optarg[1] == 'F')
                {
                    if(optarg[2] == 'T')
                    {
				        Meta.WF_TXsimulator= true;
                        printf("Simulating WF TX\n");
                    }
                    else if(optarg[2] == 'R')
                    {
				        Meta.WF_RXsimulator = true;
                        printf("Simulating WF RX\n");
                    }
                    printf("Args were WF\n");
                }
                else if( optarg[0] == 'H' && optarg[1] == 'W')
                {
                    Meta.HW_RXsimulator = true;
                    printf("Simulating HW RX\n");
                }
				break;

			case 'd':
				Meta.Debug = true;
				break;

			case 'p':
				Meta.Post= true;
				break;

			case '?':
				fatalErr("Undefined argument\n");
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	printf("Starting protocol\n");	
	
	if (Meta.Post)
    {
        testAll();
    }
	
	setup();
    simulatorSetup();

	handler();

	clean();

	exit(EXIT_SUCCESS);
}

void
setup()
{
	int rc;
    //NOTE(GoncaloXavier): Debian dependency still standing
    //TODO(GoncaloXavier): Update once protocol fixes it
	//memcpy(Self.IP,getIP(),sizeof(Self.IP)); //setting IP
	setMaster();

	Self.OutboundQueue = newQueue();
	Self.InboundQueue = newQueue();
	Self.InternalQueue = newQueue();


	Meta.Input_socket = newSocket(INBOUND_PORT);
	startSocket(Meta.Input_socket);

	Meta.Output_socket = newSocket(OUTBOUND_PORT);
	startSocket(Meta.Output_socket);
 
    if(rc = pthread_create(&(Meta.WF_listener_t), NULL, WF_listener, NULL))
    {
        fatalErr("Error: Unable to create thread, %d\n", rc);
    }

    if(rc = pthread_create(&(Meta.WF_dispatcher_t), NULL, WF_dispatcher, NULL))
    {
        fatalErr("Error: Unable to create thread, %d\n", rc);
    }
    
    if(rc = pthread_create(&(Meta.WS_listener_t), NULL, WS_listener, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if(rc = pthread_create(&(Meta.HW_dispatcher_t), NULL, HW_dispatcher, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
}

void
handler()
{
	void* Message;

	while (1)
    {
		Message = getMessage();
		if(Message == NULL) continue;
		switch (((byte*)Message)[0])
        {
			case SD:
				handleSD(Message);
				break;
			case PB:
				handlePB(Message);
				break;
			case PR:
				handlePR(Message);
				break;
			default:
				printf("Unrecognized Message type %d\n", ((byte*)Message)[0]);
		}
		free(Message);
	}
}

void
clean()
{
	if(Meta.Input_socket->s != -1)
	{
		close(Meta.Input_socket->s);
	}
	if(Meta.Output_socket->s != -1)
	{
		close(Meta.Input_socket->s);
	}
	if(Meta.Log)
    {
        fclose(Meta.Log);
    }
}

