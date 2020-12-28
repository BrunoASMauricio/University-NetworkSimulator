
#include "udp.h"

socket_s* newServerSocket()
{
	int opt;
	socklen_t dummy;
	struct sockaddr_in client, server;
	socket_s* ret = (socket_s*)malloc(sizeof(socket_s));
	if((ret->s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fatalErr("Could not creat socket\n");
	}

	ret->sock_len = sizeof(ret->sockaddr);
	memset((char *) &(ret->sockaddr), 0, ret->sock_len);
	ret->sockaddr.sin_family      = AF_INET;
	ret->sockaddr.sin_port       = 0;         // Any available port
	ret->sockaddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(ret->s, (struct sockaddr *)&(ret->sockaddr), sizeof(ret->sockaddr)) < 0)
	{
		fatalErr("Could not bind\n");
	}
	dummy = sizeof(ret->sockaddr);
	if(getsockname(ret->s, (struct sockaddr *)&(ret->sockaddr), &dummy) < 0)
	{
		fatalErr("Could not get socket info\n");
	}
	ret->port = (unsigned)htons(ret->sockaddr.sin_port);
	/*
    opt = fcntl(ret->s, F_GETFL);
    if (opt < 0) {
        fatalErr("fcntl(F_GETFL) fail.");
    }
    opt |= O_NONBLOCK;
    if (fcntl(ret->s, F_SETFL, opt) < 0) {
        fatalErr("fcntl(F_SETFL) fail.");
    }
	*/
	return ret;
}

socket_s* newSocket(unsigned short port)
{
	struct sockaddr_in servaddr;
	socket_s* ret = (socket_s*)malloc(sizeof(socket_s));
	ret->s = -1;
	ret->port = port;
	ret->sock_len = sizeof(servaddr);

	memset((char *) &(ret->sockaddr), 0, ret->sock_len);
	ret->sockaddr.sin_family = AF_INET;
	ret->sockaddr.sin_port = htons(port);
	return ret;
}

void startSocket(socket_s* sk)
{
	if ((sk->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		fprintf(stdout, "Could not generate socket errno: erro\n");
		exit(EXIT_FAILURE);
	}
	
	if (inet_aton("127.0.0.1" , &(sk->sockaddr.sin_addr)) == 0)
	{
		fprintf(stdout, "Could not connect to WF on port %d",sk->port);
		exit(EXIT_FAILURE);
	}
	/*
	if(bind(sk->s, (struct sockaddr *)&(sk->sockaddr), sizeof(sk->sockaddr)) < 0)
	{
		printf("Could not bind to %u\n", sk->port);
		exit(EXIT_FAILURE);
	}
	*/
}
int sendToSocket(socket_s* sk, void* buff, int size)
{
	return sendto(sk->s, buff, size, 0, (struct sockaddr*) &(sk->sockaddr), sk->sock_len);
}

int getFromSocket(socket_s* sk, void* buff)
{
	return recvfrom(sk->s, buff, MAX_TRANS_SIZE, 0, (struct sockaddr*) &(sk->sockaddr), &(sk->sock_len));
}
