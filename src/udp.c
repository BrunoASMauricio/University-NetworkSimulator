
#include "udp.h"

socket_s* newServerSocket()
{
	socklen_t dummy;
	struct sockaddr_in client, server;
	socket_s* ret = (socket_s*)malloc(sizeof(socket_s));
	if((ret->s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fatalErr("Could not creat socket\n");
	}

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
	ret->port = ret->sockaddr.sin_port;
	return ret;
}
/*
   if(recvfrom(s, buf, sizeof(buf), 0, NULL, NULL) <0)
   {
       tcperror("recvfrom()");
       exit(4);
   }
   close(s);
*/

socket_s* newSocket(int port)
{
	socket_s* ret = (socket_s*)malloc(sizeof(socket_s));
	ret->s = -1;
	ret->port = port;
	ret->sock_len = sizeof(sockaddr_in);

	memset((char *) &(ret->sockaddr), 0, ret->sock_len);
	ret->sockaddr.sin_family = AF_INET;
	ret->sockaddr.sin_port = htons(port);
	return ret;
}

void startSocket(socket_s* sk)
{
	if ((sk->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		fatalErr("Could not generate socket errno: %d\n", errno);
	}
	
	if (inet_aton("127.0.0.1" , &(sk->sockaddr.sin_addr)) == 0)
	{
		fatalErr("Could not connect to WF on port %d",sk->port);
	}
}

void sendToSocket(socket_s* sk, void* buff, int size)
{
	sendto(sk->s, buff, size, 0, (struct sockaddr*) &(sk->sockaddr), sk->sock_len);
}

int getFromSocket(socket_s* sk, void* buff)
{
	return recvfrom(sk->s, buff, MAX_TRANS_SIZE, 0, (struct sockaddr*) &(sk->sockaddr), &(sk->sock_len));
}
