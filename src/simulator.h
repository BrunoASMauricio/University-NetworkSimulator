//Simulate packet's COMING from WF
//Coordenate with WF team how they plan connectivity (internal sockets?)
void* 
WF_TXsimulator(void* dummy);

//Simulate packet's GOING to WF
void* 
WF_RXsimulator(void* dummy);

//Simulate packet's COMING from WS
//Coordenate with WS team how they plan connectivity and format 
void*
WS_TXsimulator(void* dummy);

//Simulate packet's GOING to HW
//Coordenate with HW team how they plan connectivity and format 
void*
HW_RXsimulator(void* dummy);
