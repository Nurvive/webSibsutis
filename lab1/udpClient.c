#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFLEN 81

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in servAddr, clientAddr;
  struct hostent *hp;
  socklen_t addrLength;
  char buf[BUFLEN];
  if (argc < 4) {
    printf("ВВЕСТИ udpclient имя_хоста порт число\n");
    exit(1);
  }
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("He получен socket\n");
    exit(1);
  }

  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  bcopy(hp->h_addr, &servAddr.sin_addr, hp->h_length);
  servAddr.sin_port = htons(atoi(argv[2]));

  bzero((char *)&clientAddr, sizeof(clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientAddr.sin_port = 0;
  if (bind(sock,(struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0) {
    perror("Клиент не получил порт.\n");
    exit(1);
  }
  printf("CLIENT: Готов к пересылке\n");
  for (int i = 0; i < atoi(argv[3]); i++){
    if (sendto(sock, argv[3], strlen(argv[3]), 0,(struct sockaddr *) &servAddr, sizeof(servAddr)) <
        0) {
    perror("Проблема с sendto.\n");
    exit(1);
    }

    bzero(buf, BUFLEN);
    addrLength = sizeof(servAddr);
    if (recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &servAddr, &addrLength) < 0) {
      perror("Плохой socket сервера.");
      exit(1);
    }
    printf("CLIENT: Сообщение с сервера - %s\n", buf);
    
    sleep(atoi(argv[3]));
  }

  printf("CLIENT: Пересылка закончена. Счастливо.\n");
  close(sock);
}
