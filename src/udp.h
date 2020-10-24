#ifndef UDP
#define UDP

#define INBOUND_PORT 2020
#define OUTBOUND_PORT 2021
#define MAX_TRANS_SIZE 2048

#include<arpa/inet.h>
#include<sys/socket.h>

typedef struct{
	int s;
	struct sockaddr_in sockaddr;
	socklen_t sock_len;
	int port;
}socket_s;

socket_s* newSocket(int port);
void startSocket(socket_s* sk);

#endif
