#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFLEN 64

#define PORT 4444

int main(){

	int clientSocket;
	struct sockaddr_in serverAddr;
	char buffer[BUFLEN];

	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("КЛИЕНТ: нe могу получить сокет.\n");
		exit(1);
	}

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		printf("КЛИЕНТ: не могу соединиться.\n");
		exit(1);
	}
	bzero(buffer, BUFLEN);
	recv(clientSocket, buffer, BUFLEN, 0);
	printf("%s\n", buffer);
	bzero(buffer, BUFLEN);
	printf("Камень - 1, ножницы - 2, бумага - 3\n");
	while(1){
		printf("Твой выбор	");
		scanf("%s", buffer);
		send(clientSocket, buffer, strlen(buffer), 0);

		if(strcmp(buffer, "-1") == 0){
			close(clientSocket);
			printf("КЛИЕНТ: Отключение от сервера.\n");
			exit(1);
		}

		if(recv(clientSocket, buffer, BUFLEN, 0) < 0){
			printf("КЛИЕНТ: ошибка получения данных.\n");
		}else{
			printf("СЕРВЕР: \t%s\n", buffer);
		}
	}

	return 0;
}
