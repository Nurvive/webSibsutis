#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BUFLEN 81

int main(int argc, const char *argv[]) {
  int sock, ssock;
  struct sockaddr_in servAddr;
  struct hostent *hp;

  if (argc < 4) {
    printf("ВВЕСТИ tcpclient имя_хоста порт сообщение\n");
    exit(1);
  }
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("He могу получить socket\n");
    exit(1);
  }

  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  bcopy(hp->h_addr, &servAddr.sin_addr, hp->h_length);
  servAddr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, &servAddr, sizeof(servAddr)) < 0) {
    perror("Клиент не может соединиться.\n");
    exit(1);
  }
  listen(sock, 1);

  printf("CLIENT: Готов к пересылке\n");
  if (send(sock, argv[3], strlen(argv[3]), 0) < 0) {
    perror("Проблемы с пересылкой.\n");
    exit(1);
  }

  printf("CLIENT: Пересылка завершена.\n");
  close(sock);
  exit(0);
}
