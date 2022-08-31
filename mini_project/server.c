/*
Author: Harsh Shah
Roll: MT2021050
Server side: concurrent server using fork()
socket()->bind()->listen()->accept()->recv()/send()->close()
*/

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "serverUtils/serverUtils.h"

#define PORT 50000

int main(void)
{

	int socket_desc, client_sock, c;
	struct sockaddr_in server, client;
	char buf[100];

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		printf("Could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
		perror("bind failed. Error");

	listen(socket_desc, 3);
	c = sizeof(struct sockaddr_in);

	while (1)
	{
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);

		if (!fork())
		{
			// inside child process
			close(socket_desc);
			service_cli(client_sock); // Service client, once done client exits
			exit(0);
		}
		else
			close(client_sock);
	}
	return 0;
}
