/*
Author: Harsh Shah
Roll: MT2021050
Client side:
socket()->connect()->send()/recv()->close()
*/

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "clientUtils/clientUtils.h"

#define PORT 50000

int main(void)
{
	int sock;
	struct sockaddr_in server;
	char server_reply[50], *server_ip;
	server_ip = "127.0.0.1";

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		printf("Could not create socket");

	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		perror("connect failed. Error");

	while (client(sock) != 3)
		;
	close(sock);

	return 0;
}
