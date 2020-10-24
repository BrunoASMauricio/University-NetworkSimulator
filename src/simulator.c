//Simulate packet's COMING from WF
void* 
WF_TXsimulator(void* dummy)
{
    printf("WF TX simulator started\n");
    while(1)
    {
        sleep(1);
    }
}

//Simulate packet's GOING to WF
void* 
WF_RXsimulator(void* dummy)
{
    printf("WF RX simulator started\n");
    while(1)
    {
        sleep(1);
    }
}

//Simulate packet's COMING from WS
void*
WS_TXsimulator(void* dummy)
{
    printf("WS TX simulator started\n");
    while(1)
    {
        sleep(1);
    }
}

void*
HW_RXsimulator(void* dummy)
{
    printf("HW RX simulator started\n");
    while(1)
    {
        sleep(1);
    }
}


void simulatorSetup()
{
	int rc;
    if(Meta.WF_TXsimulator == true)
    {
        if(rc = pthread_create(&(Meta.WF_dispatcher_t), NULL, WF_TXsimulator, NULL))
        {
            fatalErr("Error: Unable to create Simulator thread, %d\n", rc);
        }
    }
    
    if(Meta.WF_RXsimulator == true)
    {
        if(rc = pthread_create(&(Meta.WF_listener_t), NULL, WF_RXsimulator, NULL))
        {
            fatalErr("Error: Unable to create Simulator thread, %d\n", rc);
        }
    }
    
    if(Meta.WS_TXsimulator == true)
    {
        if(rc = pthread_create(&(Meta.WS_dispatcher_t), NULL, WS_TXsimulator, NULL))
        {
            fatalErr("Error: Unable to create Simulator thread, %d\n", rc);
        }
    }
    
    if(Meta.HW_RXsimulator == true)
    {
        if(rc = pthread_create(&(Meta.HW_listener_t), NULL, HW_RXsimulator, NULL))
        {
            fatalErr("Error: Unable to create Simulator thread, %d\n", rc);
        }
    }
}



bool
dropPacket()
{
    bool result;
    (rand()%100 <= Meta.InterferanceLevel) ? result = true : result =  false;
    
    return result;
}
